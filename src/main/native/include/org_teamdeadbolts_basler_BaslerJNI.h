/* Team Deadbolts (C) 2026 */
#include <jni.h>
/* Header for class org_teamdeadbolts_basler_BaslerJNI */

#ifndef _Included_org_teamdeadbolts_basler_BaslerJNI
#define _Included_org_teamdeadbolts_basler_BaslerJNI
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    isLibraryWorking
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_isLibraryWorking(JNIEnv *, jclass);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    isCameraRemoved
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_isCameraRemoved(JNIEnv *, jclass,
                                                        jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    cleanUp
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_cleanUp(JNIEnv *,
                                                                       jclass);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getCameraModelRaw
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getCameraModelRaw(JNIEnv *, jclass,
                                                          jstring);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getConnectedCamerasInternal
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getConnectedCamerasInternal(JNIEnv *,
                                                                    jclass);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    createCameraInternal
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_createCameraInternal(JNIEnv *, jclass,
                                                             jstring);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    startCameraInternal
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_startCameraInternal(JNIEnv *, jclass,
                                                            jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    stopCameraInternal
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_stopCameraInternal(JNIEnv *, jclass,
                                                           jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    destroyCameraInternal
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_destroyCameraInternal(JNIEnv *, jclass,
                                                              jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setExposureInternal
 * Signature: (JD)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setExposureInternal(JNIEnv *, jclass,
                                                            jlong, jdouble);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setGainInternal
 * Signature: (JD)I
 */
JNIEXPORT jint JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_setGainInternal(
    JNIEnv *, jclass, jlong, jdouble);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setAutoExposureInternal
 * Signature: (JZ)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setAutoExposureInternal(JNIEnv *,
                                                                jclass, jlong,
                                                                jboolean);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setFrameRateInternal
 * Signature: (JD)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setFrameRateInternal(JNIEnv *, jclass,
                                                             jlong, jdouble);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setWhiteBalanceInternal
 * Signature: (J[D)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setWhiteBalanceInternal(JNIEnv *,
                                                                jclass, jlong,
                                                                jdoubleArray);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setAutoWhiteBalanceInternal
 * Signature: (JZ)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setAutoWhiteBalanceInternal(JNIEnv *,
                                                                    jclass,
                                                                    jlong,
                                                                    jboolean);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setBrightnessInternal
 * Signature: (JD)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setBrightnessInternal(JNIEnv *, jclass,
                                                              jlong, jdouble);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setPixelFormatInternal
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setPixelFormatInternal(JNIEnv *, jclass,
                                                               jlong, jint);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setPixelBinningInternal
 * Signature: (JIII)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setPixelBinningInternal(JNIEnv *,
                                                                jclass, jlong,
                                                                jint, jint,
                                                                jint);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setDeviceLinkThroughputLimitEnableInternal
 * Signature: (JZ)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setDeviceLinkThroughputLimitEnableInternal(
    JNIEnv *, jclass, jlong, jboolean);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getExposureInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getExposureInternal(JNIEnv *, jclass,
                                                            jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getGainInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getGainInternal(JNIEnv *, jclass,
                                                        jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getAutoExposureInternal
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getAutoExposureInternal(JNIEnv *,
                                                                jclass, jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getFrameRateInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getFrameRateInternal(JNIEnv *, jclass,
                                                             jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getWhiteBalanceInternal
 * Signature: (J)[D
 */
JNIEXPORT jdoubleArray JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getWhiteBalanceInternal(JNIEnv *,
                                                                jclass, jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getAutoWhiteBalanceInternal
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getAutoWhiteBalanceInternal(JNIEnv *,
                                                                    jclass,
                                                                    jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getSupportedPixelFormatsInternal
 * Signature: (J)[I
 */
JNIEXPORT jintArray JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getSupportedPixelFormatsInternal(
    JNIEnv *, jclass, jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getMinExposureInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getMinExposureInternal(JNIEnv *, jclass,
                                                               jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getMaxExposureInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getMaxExposureInternal(JNIEnv *, jclass,
                                                               jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getMinWhiteBalanceInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getMinWhiteBalanceInternal(JNIEnv *,
                                                                   jclass,
                                                                   jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getMaxWhiteBalanceInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getMaxWhiteBalanceInternal(JNIEnv *,
                                                                   jclass,
                                                                   jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getMinGainInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getMinGainInternal(JNIEnv *, jclass,
                                                           jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getMaxGainInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getMaxGainInternal(JNIEnv *, jclass,
                                                           jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getLatestTimestampInternal
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getLatestTimestampInternal(JNIEnv *,
                                                                   jclass,
                                                                   jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    takeFrameInternal
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_takeFrameInternal(JNIEnv *, jclass,
                                                          jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    awaitNewFrameInternal
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_awaitNewFrameInternal(JNIEnv *, jclass,
                                                              jlong);

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getPixelFormatInternal
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getPixelFormatInternal(JNIEnv *, jclass,
                                                               jlong);

#ifdef __cplusplus
}
#endif
#endif
