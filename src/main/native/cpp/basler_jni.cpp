/* Team Deadbolts (C) 2026 */
#include <pylon/BaslerUniversalInstantCamera.h>
#include <pylon/PylonIncludes.h>

#include <atomic>
#include <map>
#include <mutex>
#include <thread>

#include "camera_instance.hpp"
#include "org_teamdeadbolts_basler_BaslerJNI.h"

using namespace Pylon;
using namespace Basler_UniversalCameraParams;

static std::map<jlong, std::shared_ptr<CameraInstance>> cMap;
static std::mutex mapMutex;
static bool pylonInit = false;

std::string jstringToString(JNIEnv *env, jstring jStr) {
  if (!jStr) return "";
  const char *chars = env->GetStringUTFChars(jStr, nullptr);
  std::string str(chars);
  env->ReleaseStringUTFChars(jStr, chars);
  return str;
}

std::shared_ptr<CameraInstance> getCameraInstance(jlong handle) {
  std::lock_guard<std::mutex> lock(mapMutex);
  auto it = cMap.find(handle);
  if (it != cMap.end()) {
    return it->second;
  }
  return nullptr;
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    isLibraryWorking
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_isLibraryWorking(JNIEnv *env, jclass) {
  try {
    std::lock_guard<std::mutex> lock(mapMutex);
    if (!pylonInit) {
      PylonInitialize();
      pylonInit = true;
    }
    return JNI_TRUE;
  } catch (const GenericException &) {
    return JNI_FALSE;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getCameraModelRaw
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getCameraModelRaw(
    JNIEnv *env, jclass, jstring serialNumber) {
  try {
    {
      std::lock_guard<std::mutex> lock(mapMutex);
      if (!pylonInit) {
        PylonInitialize();
        pylonInit = true;
      }
    }

    std::string serial = jstringToString(env, serialNumber);
    CTlFactory &tlFactory = CTlFactory::GetInstance();
    DeviceInfoList_t devices;

    if (tlFactory.EnumerateDevices(devices) > 0) {
      for (const auto &devInfo : devices) {
        if (std::string(devInfo.GetSerialNumber()) == serial) {
          std::string modelName(devInfo.GetModelName());
          return env->NewStringUTF(modelName.c_str());
        }
      }
    }

    return env->NewStringUTF("UNKNOWN");
  } catch (const GenericException &) {
    return env->NewStringUTF("UNKNOWN");
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getConnectedCamerasInternal
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getConnectedCamerasInternal(JNIEnv *env,
                                                                    jclass) {
  try {
    {
      std::lock_guard<std::mutex> lock(mapMutex);
      if (!pylonInit) {
        PylonInitialize();
        pylonInit = true;
      }
    }

    CTlFactory &tlFactory = CTlFactory::GetInstance();
    DeviceInfoList_t devices;
    size_t numDevices = tlFactory.EnumerateDevices(devices);

    jclass stringClass = env->FindClass("java/lang/String");
    if (stringClass == nullptr) {
      env->ExceptionClear();
      return nullptr;
    }

    jobjectArray result = env->NewObjectArray(numDevices, stringClass, nullptr);
    if (result == nullptr) {
      env->ExceptionClear();
      return nullptr;
    }

    for (size_t i = 0; i < numDevices; i++) {
      const char *serialCStr = devices[i].GetSerialNumber();
      if (!serialCStr) continue;

      jstring jSerial = env->NewStringUTF(serialCStr);
      if (!jSerial) {
        env->ExceptionClear();
        continue;
      }

      env->SetObjectArrayElement(result, i, jSerial);
      env->DeleteLocalRef(jSerial);
    }

    return result;
  } catch (const GenericException &e) {
    std::cout << "Pylon exception: " << e.GetDescription() << std::endl;
    return nullptr;
  } catch (...) {
    std::cout << "Unknown exception in getConnectedCameras" << std::endl;
    return nullptr;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    createCameraInternal
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_createCameraInternal(
    JNIEnv *env, jclass, jstring serialNumber) {
  try {
    {
      std::lock_guard<std::mutex> lock(mapMutex);
      if (!pylonInit) {
        PylonInitialize();
        pylonInit = true;
      }
    }

    std::string serial = jstringToString(env, serialNumber);
    CTlFactory &tlFactory = CTlFactory::GetInstance();

    CDeviceInfo devInfo;
    devInfo.SetSerialNumber(serial.c_str());

    IPylonDevice *device = tlFactory.CreateDevice(devInfo);
    auto instance = std::make_shared<CameraInstance>(device);

    jlong handle = reinterpret_cast<jlong>(instance.get());

    {
      std::lock_guard<std::mutex> lock(mapMutex);
      cMap[handle] = instance;
    }

    return handle;
  } catch (const GenericException &) {
    return 0;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    startCameraInternal
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_startCameraInternal(JNIEnv *env, jclass,
                                                            jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) return JNI_FALSE;

  return instance->start() ? JNI_TRUE : JNI_FALSE;
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    stopCameraInternal
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_stopCameraInternal(JNIEnv *env, jclass,
                                                           jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) return JNI_FALSE;

  return instance->stop() ? JNI_TRUE : JNI_FALSE;
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    destroyCameraInternal
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_destroyCameraInternal(JNIEnv *env,
                                                              jclass,
                                                              jlong handle) {
  {
    std::lock_guard<std::mutex> lock(mapMutex);
    cMap.erase(handle);
  }

  return JNI_TRUE;
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setExposureInternal
 * Signature: (JD)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setExposureInternal(JNIEnv *env, jclass,
                                                            jlong handle,
                                                            jdouble exposure) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -2;

  return instance->setExposure(exposure);
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setGainInternal
 * Signature: (JD)I
 */
JNIEXPORT jint JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_setGainInternal(
    JNIEnv *env, jclass, jlong handle, jdouble gain) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -2;

  return instance->setGain(gain);
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setAutoExposureInternal
 * Signature: (JZ)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setAutoExposureInternal(
    JNIEnv *env, jclass, jlong handle, jboolean enable) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -2;

  return instance->setAutoExposure(enable);
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setFrameRateInternal
 * Signature: (JD)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setFrameRateInternal(
    JNIEnv *env, jclass, jlong handle, jdouble frameRate) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -2;

  return instance->setFrameRate(frameRate);
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setWhiteBalanceInternal
 * Signature: (J[D)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setWhiteBalanceInternal(
    JNIEnv *env, jclass, jlong handle, jdoubleArray rgb) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -2;

  jsize length = env->GetArrayLength(rgb);
  if (length != 3) {
    std::cout << "Expected array of length 3 for RGB balance, got " << length
              << std::endl;
    return -1;
  }

  jdouble buffer[3];
  env->GetDoubleArrayRegion(rgb, 0, 3, buffer);

  return instance->setWhiteBalance({buffer[0], buffer[1], buffer[2]});
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setAutoWhiteBalanceInternal
 * Signature: (JZ)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setAutoWhiteBalanceInternal(
    JNIEnv *env, jclass, jlong handle, jboolean enable) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -2;

  return instance->setAutoWhiteBalance(enable);
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setBrightnessInternal
 * Signature: (JD)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setBrightnessInternal(
    JNIEnv *, jclass, jlong handle, jdouble brightness) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -2;

  return instance->setBrightness(brightness);
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setPixelFormatInternal
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setPixelFormatInternal(JNIEnv *, jclass,
                                                               jlong handle,
                                                               jint format) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -2;

  return instance->setPixelFormat(format);
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setPixelBinningInternal
 * Signature: (JIII)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setPixelBinningInternal(
    JNIEnv *, jclass, jlong handle, jint mode, jint horzBin, jint vertBin) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -2;

  return instance->setPixelBinning(mode, horzBin, vertBin) ? 0 : -1;
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setDeviceLinkThroughputLimitEnableInternal
 * Signature: (JZ)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setDeviceLinkThroughputLimitEnableInternal(
    JNIEnv *, jclass, jlong handle, jboolean enable) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -2;

  return instance->setDeviceLinkThroughputLimitEnable(enable);
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getExposureInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getExposureInternal(JNIEnv *env, jclass,
                                                            jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -1.0;

  return instance->getExposure();
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getGainInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getGainInternal(JNIEnv *env, jclass,
                                                        jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -1.0;

  return instance->getGain();
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getAutoExposureInternal
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getAutoExposureInternal(JNIEnv *env,
                                                                jclass,
                                                                jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -1;

  // 1 = True, 0 = False
  return instance->getAutoExposure();
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getFrameRateInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getFrameRateInternal(JNIEnv *env,
                                                             jclass,
                                                             jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -1.0;

  return instance->getFrameRate();
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getWhiteBalanceInternal
 * Signature: (J)[D
 */
JNIEXPORT jdoubleArray JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getWhiteBalanceInternal(JNIEnv *env,
                                                                jclass,
                                                                jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) return nullptr;

  std::array<double, 3> balance = instance->getWhiteBalance();
  jdoubleArray result = env->NewDoubleArray(3);
  if (!result) return nullptr;

  env->SetDoubleArrayRegion(result, 0, 3, balance.data());
  return result;
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getAutoWhiteBalanceInternal
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getAutoWhiteBalanceInternal(
    JNIEnv *env, jclass, jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -1;

  return instance->getAutoWhiteBalance();
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getSupportedPixelFormatsInternal
 * Signature: (J)[I
 */
JNIEXPORT jintArray JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getSupportedPixelFormatsInternal(
    JNIEnv *env, jclass, jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) return nullptr;

  std::vector<int> formats = instance->getSupportedPixelFormats();
  jintArray result = env->NewIntArray(formats.size());
  if (!result) return nullptr;

  env->SetIntArrayRegion(result, 0, formats.size(), formats.data());
  return result;
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getPixelFormatInternal
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getPixelFormatInternal(JNIEnv *env,
                                                               jclass,
                                                               jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -1;
  return instance->getPixelFormat();
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getMinExposureInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getMinExposureInternal(JNIEnv *, jclass,
                                                               jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) {
    return -1.0;
  }
  return instance->getMinExposure();
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getMaxExposureInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getMaxExposureInternal(JNIEnv *, jclass,
                                                               jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) {
    return -1.0;
  }
  return instance->getMaxExposure();
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getMinWhiteBalanceInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getMinWhiteBalanceInternal(
    JNIEnv *, jclass, jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) {
    return -1.0;
  }
  return instance->getMinWhiteBalance();
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getMaxWhiteBalanceInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getMaxWhiteBalanceInternal(
    JNIEnv *, jclass, jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) {
    return -1.0;
  }
  return instance->getMaxWhiteBalance();
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getMinGainInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getMinGainInternal(JNIEnv *, jclass,
                                                           jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -1.0;
  return instance->getMinGain();
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getMaxGainInternal
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getMaxGainInternal(JNIEnv *, jclass,
                                                           jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -1.0;
  return instance->getMaxGain();
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getLatestTimestampInternal
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getLatestTimestampInternal(
    JNIEnv *env, jclass, jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) return 0;

  return static_cast<jlong>(instance->getLatestTimestamp());
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    takeFrameInternal
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_takeFrameInternal(JNIEnv *env, jclass,
                                                          jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) return 0;

  auto matPtr = instance->takeFrame();
  if (!matPtr) return 0;

  if (matPtr->empty() || matPtr->cols <= 0 || matPtr->rows <= 0) {
    return 0;
  }

  cv::Mat *javaMat = new cv::Mat(*matPtr);
  return reinterpret_cast<jlong>(javaMat);
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    isCameraRemoved
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_isCameraRemoved(JNIEnv *env, jclass,
                                                        jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) return JNI_TRUE;  // Assume true if the handle is invalid

  return instance->isRemoved() ? JNI_TRUE : JNI_FALSE;
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    awaitNewFrameInternal
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_awaitNewFrameInternal(JNIEnv *env,
                                                              jclass,
                                                              jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) return -1;  // -1 for error

  return instance->awaitNewFrame();
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    cleanUp
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_cleanUp(JNIEnv *,
                                                                       jclass) {
  {
    std::lock_guard<std::mutex> lock(mapMutex);
    cMap.clear();
  }
  if (pylonInit) {
    PylonTerminate();
    pylonInit = false;
  }
}
