# ownCloud Desktop Client setup

#### Official setup document: https://doc.owncloud.com/desktop/next/appendices/building.html.
#### Official github repository: https://github.com/owncloud/client.

## Setting up KDE Craft

To install KDE Craft, Python 3.6+, Microsoft Visual Studio 2019 and PowerShell 5.0+ must be installed.

For Microsoft Visual Studio 2019, make sure the following components are selected at the minimum:
*	Desktop Development with C++
*	C++ ATL 
*	Windows SDK

## Install KDE Craft

Open PowerShell as admin and run the following:
1.	Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
2.	iex ((new-object net.webclient).DownloadString('https://raw.githubusercontent.com/KDE/craft/master/setup/install_craft.ps1'))

**IMPORTANT**: Choose Microsoft Visual Studio 2019 as the compiler but for everything else accept the defaults.

## Launch the KDE Craft Environment

3.	C:\CraftRoot\craft\craftenv.ps1
4.	craft nsis
5.	craft -i libs/zlib
6.	craft -i libs/openssl
7.	craft --no-cache -i libs/libcurl
8.	Open "C:\CraftRoot\build\libs\libcurl\work\curl-7.78.0\CMakeLists.txt" 
find
cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
after it add
set(CMAKE_USE_OPENSSL ON)
9.	craft --compile libs/libcurl
10.	craft --add-blueprint-repository https://github.com/filesfm/craft-blueprints-owncloud.git
11.	craft owncloud-client

## Commands for developing

#### Open PowerShell as admin
*	C:\CraftRoot\craft\craftenv.ps1 (allows to work with craft)
#### If everything is up to date:
*	craft owncloud-client
#### If changes have been made in the working branch:
*	craft --install-deps owncloud-client
*	craft --fetch owncloud-client
*	craft --configure --make --install 
*	craft owncloud-client
#### To switch branches(for example to build the 2.10 branch):
*	git checkout 2.10
*	craft --set version=2.10 owncloud-client

#### To compile locally modified changes:
*	craft --compile --install --qmerge owncloud-client
_The compiled result can be found under "C:\CraftRoot\build\owncloud\owncloud-client\work\build\bin" and changes tested from C:\CraftRoot\bin_

#### To create an installer execute: 
*	craft --package owncloud-client
_The result can be found under "C:\CraftRoot\tmp"_

# Local code signing

1)	Edit "C:\CraftRoot\etc\CraftSettings.ini", navigate to [CodeSigning] and change it to:

```
Enabled = True
Protected = False
SignCache = ${CodeSigning:Enabled}
CommonName = Files.fm
Organization = Files.fm
Street = 
Locality = Riga
Country = LV
State = 
PostalCode =

```
2)	Run this script in PowerShell as admin:

``` 

#
# This script will create and install two certificates:
#     1. `MyCA.cer`: A self-signed root authority certificate. 
#     2. `MySPC.cer`: The cerificate to sign code in 
#         a development environment (signed with `MyCA.cer`).
# 
# No user interaction is needed (unattended). 
# Powershell 4.0 or higher is required.
#

# Define the expiration date for certificates.
$notAfter = (Get-Date).AddYears(10)

# Create a self-signed root Certificate Authority (CA).
$rootCert = New-SelfSignedCertificate -KeyExportPolicy Exportable -CertStoreLocation Cert:\CurrentUser\My -DnsName "My CA" -NotAfter $notAfter -TextExtension @("2.5.29.37={text}1.3.6.1.5.5.7.3.3", "2.5.29.19={text}CA=1") -KeyusageProperty All -KeyUsage CertSign, CRLSign, DigitalSignature

# Export the CA private key.
[System.Security.SecureString] $password = ConvertTo-SecureString -String "passwordx" -Force -AsPlainText
[String] $rootCertPath = Join-Path -Path cert:\CurrentUser\My\ -ChildPath "$($rootcert.Thumbprint)"
Export-PfxCertificate -Cert $rootCertPath -FilePath "MyCA.pfx" -Password $password
Export-Certificate -Cert $rootCertPath -FilePath "MyCA.crt"

# Create an end certificate signed by our CA.
$cert = New-SelfSignedCertificate -CertStoreLocation Cert:\LocalMachine\My -DnsName "Files.fm" -NotAfter $notAfter -Signer $rootCert -Type CodeSigningCert -TextExtension @("2.5.29.37={text}1.3.6.1.5.5.7.3.3", "2.5.29.19={text}CA=0&pathlength=0")

# Save the signed certificate with private key into a PFX file and just the public key into a CRT file.
[String] $certPath = Join-Path -Path cert:\LocalMachine\My\ -ChildPath "$($cert.Thumbprint)"
Export-PfxCertificate -Cert $certPath -FilePath "MySPC.pfx" -Password $password
Export-Certificate -Cert $certPath -FilePath "MySPC.crt"

# Add MyCA certificate to the Trusted Root Certification Authorities.
$pfx = new-object System.Security.Cryptography.X509Certificates.X509Certificate2
$pfx.import("MyCA.pfx", $password, "Exportable,PersistKeySet")
$store = new-object System.Security.Cryptography.X509Certificates.X509Store(
    [System.Security.Cryptography.X509Certificates.StoreName]::Root,
    "localmachine"
)
$store.open("MaxAllowed")
$store.add($pfx)
$store.close()

# Remove MyCA from CurrentUser to avoid issues when signing with "signtool.exe /a ..."
Remove-Item -Force "cert:\CurrentUser\My\$($rootCert.Thumbprint)"

# Import certificate.
Import-PfxCertificate -FilePath MySPC.pfx cert:\CurrentUser\My -Password $password -Exportable

```

3)	Rename MyCA.pfx to mycert.pfx and place it in C:\CraftRoot
