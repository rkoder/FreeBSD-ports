Address CVE-2024-36048.

https://www.qt.io/blog/security-advisory-qstringconverter-0

--- src/oauth/qabstractoauth.cpp.orig	2024-01-04 19:21:59 UTC
+++ src/oauth/qabstractoauth.cpp
@@ -37,7 +37,6 @@
 #include <QtCore/qurl.h>
 #include <QtCore/qpair.h>
 #include <QtCore/qstring.h>
-#include <QtCore/qdatetime.h>
 #include <QtCore/qurlquery.h>
 #include <QtCore/qjsondocument.h>
 #include <QtCore/qmessageauthenticationcode.h>
@@ -46,6 +45,9 @@
 #include <QtNetwork/qnetworkaccessmanager.h>
 #include <QtNetwork/qnetworkreply.h>
 
+#include <QtCore/qrandom.h>
+#include <QtCore/private/qlocking_p.h>
+
 #include <random>
 
 Q_DECLARE_METATYPE(QAbstractOAuth::Error)
@@ -290,15 +292,19 @@ void QAbstractOAuthPrivate::setStatus(QAbstractOAuth::
     }
 }
 
+static QBasicMutex prngMutex;
+Q_GLOBAL_STATIC_WITH_ARGS(std::mt19937, prng, (*QRandomGenerator::system()))
+
 QByteArray QAbstractOAuthPrivate::generateRandomString(quint8 length)
 {
-    const char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
-    static std::mt19937 randomEngine(QDateTime::currentDateTime().toMSecsSinceEpoch());
+    constexpr char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
     std::uniform_int_distribution<int> distribution(0, sizeof(characters) - 2);
     QByteArray data;
     data.reserve(length);
+    auto lock = qt_unique_lock(prngMutex);
     for (quint8 i = 0; i < length; ++i)
-        data.append(characters[distribution(randomEngine)]);
+        data.append(characters[distribution(*prng)]);
+    lock.unlock();
     return data;
 }
 
@@ -614,6 +620,7 @@ void QAbstractOAuth::resourceOwnerAuthorization(const 
 }
 
 /*!
+    \threadsafe
     Generates a random string which could be used as state or nonce.
     The parameter \a length determines the size of the generated
     string.
