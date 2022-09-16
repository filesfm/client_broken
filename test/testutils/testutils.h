#pragma once

#include "account.h"
#include "folder.h"
#include "folderman.h"

namespace OCC {

namespace TestUtils {
    FolderMan *folderMan();
    FolderDefinition createDummyFolderDefinition(const QString &path);
    AccountPtr createDummyAccount();
}
}
