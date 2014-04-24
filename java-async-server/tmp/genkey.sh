#!/bin/sh

# gen the self signed certificate and key pair
rm keys
keytool -genkeypair -alias moo -keyalg RSA -validity 100 -keystore keys <<EOF
welcome
welcome
Captain Jack Sparrow
Server, Framework
Saju Pillai
Bangalore
Karnataka
IN
yes
welcome
EOF

# show it to the user
keytool -list -v -keystore keys <<EOF
welcome
EOF
