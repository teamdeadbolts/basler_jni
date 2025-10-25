# Basler JNI
This is a JNI to bridge the Basler Pylon SDK to Java. It is really only designed to be used with PhotonVision.

## Building
You need to install [Pylon 7.5](https://www.baslerweb.com/en-us/downloads/software/3359722533/?downloadCategory.values.label.data=pylon) first.
```bash
$ ./gradlew build --PpylonRoot=[path to pylon install]
```