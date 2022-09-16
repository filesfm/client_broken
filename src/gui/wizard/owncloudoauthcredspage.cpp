/*
 * Copyright (C) by Olivier Goffart <ogoffart@woboq.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include <QVariant>
#include <QMenu>
#include <QClipboard>

#include "application.h"
#include "wizard/owncloudoauthcredspage.h"
#include "theme.h"
#include "account.h"
#include "cookiejar.h"
#include "settingsdialog.h"
#include "wizard/owncloudwizardcommon.h"
#include "wizard/owncloudwizard.h"
#include "creds/httpcredentialsgui.h"

namespace OCC {

OwncloudOAuthCredsPage::OwncloudOAuthCredsPage()
    : AbstractCredentialsWizardPage()
{
    _ui.setupUi(this);

    _ui.topLabel->hide();
    _ui.bottomLabel->hide();

    WizardCommon::initErrorLabel(_ui.errorLabel);

    setTitle(WizardCommon::titleTemplate().arg(tr("Connect to %1").arg(Theme::instance()->appNameGUI())));
    setSubTitle(WizardCommon::subTitleTemplate().arg(tr("Login in your browser")));

    connect(_ui.openLinkButton, &QCommandLinkButton::clicked, [this] {
        _ui.errorLabel->hide();
        oauth()->openBrowser();
    });
    _ui.openLinkButton->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(_ui.openLinkButton, &QWidget::customContextMenuRequested, [this](const QPoint &pos) {
        auto menu = new QMenu(_ui.openLinkButton);
        menu->addAction(tr("Copy link to clipboard"), this, [this] {
            oauth()->authorisationLinkAsync([](const QUrl &link) {
                QApplication::clipboard()->setText(link.toString(QUrl::FullyEncoded));
            });
        });
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->popup(_ui.openLinkButton->mapToGlobal(pos));
    });
}

void OwncloudOAuthCredsPage::initializePage()
{
    OwncloudWizard *ocWizard = qobject_cast<OwncloudWizard *>(wizard());
    Q_ASSERT(ocWizard);
    ocWizard->account()->setCredentials(new HttpCredentialsGui);

    oauth();
    ocApp()->gui()->settingsDialog()->showMinimized();
}

void OCC::OwncloudOAuthCredsPage::cleanupPage()
{
    // The next or back button was activated, show the wizard again
    wizard()->showNormal();
    _asyncAuth.reset();
}

void OwncloudOAuthCredsPage::asyncAuthResult(OAuth::Result r, const QString &user,
    const QString &token, const QString &refreshToken)
{
    _asyncAuth.reset();
    switch (r) {
    case OAuth::NotSupported: {
        /* OAuth not supported (can't open browser), fallback to HTTP credentials */
        owncloudWizard()->back();
        owncloudWizard()->setAuthType(DetermineAuthTypeJob::AuthType::Basic);
        break;
    }
    case OAuth::Error:
        /* Error while getting the access token.  (Timeout, or the server did not accept our client credentials */
        _ui.errorLabel->show();
        wizard()->showNormal();
        break;
    case OAuth::LoggedIn: {
        _token = token;
        _user = user;
        _refreshToken = refreshToken;
        emit connectToOCUrl(owncloudWizard()->account()->url().toString());
        if (Theme::instance()->wizardSkipAdvancedPage()) {
            emit owncloudWizard()->createLocalAndRemoteFolders();
        }
        break;
    }
    }
}

OAuth *OwncloudOAuthCredsPage::oauth()
{
    if (!_asyncAuth) {
        _asyncAuth.reset(new OAuth(owncloudWizard()->account().data(), this));
        connect(_asyncAuth.data(), &OAuth::result, this, &OwncloudOAuthCredsPage::asyncAuthResult, Qt::QueuedConnection);
        _asyncAuth->startAuthentication();
    }
    return _asyncAuth.get();
}

int OwncloudOAuthCredsPage::nextId() const
{
    if (Theme::instance()->wizardSkipAdvancedPage()) {
        return -1;
    }
    return WizardCommon::Page_AdvancedSetup;
}

void OwncloudOAuthCredsPage::setConnected()
{
    wizard()->showNormal();
}

AbstractCredentials *OwncloudOAuthCredsPage::getCredentials() const
{
    return new HttpCredentialsGui(_user, _token, _refreshToken);
}

bool OwncloudOAuthCredsPage::isComplete() const
{
    return false; /* We can never go forward manually */
}

} // namespace OCC
