#include "camera_instance.hpp"
#include "org_teamdeadbolts_basler_BaslerJNI.h"
#include <atomic>
#include <map>
#include <mutex>
#include <pylon/BaslerUniversalInstantCamera.h>
#include <pylon/PylonIncludes.h>
#include <thread>

using namespace Pylon;
using namespace Basler_UniversalCameraParams;

static std::map<jlong, std::shared_ptr<CameraInstance>> cMap;
static std::mutex mapMutex;
static bool pylonInit = false;

std::string jstringToString(JNIEnv *env, jstring jStr) {
  if (!jStr)
    return "";
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
    if (!pylonInit) {
      PylonInitialize();
      pylonInit = true;
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

    // Return null if not found
    return nullptr;
  } catch (const GenericException &) {
    return nullptr;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getConnectedCameras
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getConnectedCameras(JNIEnv *env,
                                                            jclass) {

  try {
    if (!pylonInit) {
      PylonInitialize();
      pylonInit = true;
    }

    CTlFactory &tlFactory = CTlFactory::GetInstance();
    DeviceInfoList_t devices;
    size_t numDevices = tlFactory.EnumerateDevices(devices);

    jclass stringClass = env->FindClass("java/lang/String");
    if (stringClass == nullptr) {
      env->ExceptionClear(); // clear pending exception
      return nullptr;
    }

    jobjectArray result = env->NewObjectArray(numDevices, stringClass, nullptr);
    if (result == nullptr) {
      env->ExceptionClear();
      return nullptr;
    }

    for (size_t i = 0; i < numDevices; i++) {
      const char *serialCStr = devices[i].GetSerialNumber();
      if (!serialCStr)
        continue; // skip invalid devices

      jstring jSerial = env->NewStringUTF(serialCStr);
      if (!jSerial) {
        env->ExceptionClear(); // skip if UTF conversion fails
        continue;
      }

      env->SetObjectArrayElement(result, i, jSerial);
      env->DeleteLocalRef(jSerial);
    }

    return result;
  } catch (const GenericException &e) {
    std::cerr << "Pylon exception: " << e.GetDescription() << std::endl;
    return nullptr;
  } catch (...) {
    std::cerr << "Unknown exception in getConnectedCameras" << std::endl;
    return nullptr;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    createCamera
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_createCamera(
    JNIEnv *env, jclass, jstring serialNumber) {
  try {
    if (!pylonInit) {
      PylonInitialize();
      pylonInit = true;
    }

    std::string serial = jstringToString(env, serialNumber);
    CTlFactory &tlFactory = CTlFactory::GetInstance();

    CDeviceInfo devInfo;
    devInfo.SetSerialNumber(serial.c_str());

    IPylonDevice *device = tlFactory.CreateDevice(devInfo);
    auto instance = std::make_shared<CameraInstance>(device);
    // instance->camera->Open();

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
 * Method:    startCamera
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_startCamera(
    JNIEnv *env, jclass, jlong handle) {
  try {
    auto instance = getCameraInstance(handle);
    if (!instance)
      return JNI_FALSE;

    instance->start();
    return JNI_TRUE;
  } catch (const GenericException &) {
    return JNI_FALSE;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    stopCamera
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_stopCamera(
    JNIEnv *env, jclass, jlong handle) {
  try {
    auto instance = getCameraInstance(handle);
    if (!instance)
      return JNI_FALSE;

    instance->stop();
    return JNI_TRUE;
  } catch (const GenericException &) {
    return JNI_FALSE;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    destroyCamera
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_destroyCamera(JNIEnv *env, jclass,
                                                      jlong handle) {
  {
    std::lock_guard<std::mutex> lock(mapMutex);
    cMap.erase(handle);
  }

  return JNI_TRUE;
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setExposure
 * Signature: (JD)Z
 */
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_setExposure(
    JNIEnv *env, jclass, jlong handle, jdouble exposure) {
  try {
    auto instance = getCameraInstance(handle);
    if (!instance)
      return JNI_FALSE;

    instance->setExposure(exposure);
    return JNI_TRUE;
  } catch (const GenericException &) {
    return JNI_FALSE;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setGain
 * Signature: (JD)Z
 */
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_setGain(
    JNIEnv *env, jclass, jlong handle, jdouble gain) {
  try {
    auto instance = getCameraInstance(handle);
    if (!instance)
      return JNI_FALSE;

    instance->setGain(gain);
    return JNI_TRUE;
  } catch (const GenericException &) {
    return JNI_FALSE;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setAutoExposure
 * Signature: (JZ)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setAutoExposure(JNIEnv *env, jclass,
                                                        jlong handle,
                                                        jboolean enable) {
  try {
    auto instance = getCameraInstance(handle);
    if (!instance)
      return JNI_FALSE;

    instance->setAutoExposure(enable);
    return JNI_TRUE;
  } catch (const GenericException &) {
    return JNI_FALSE;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setFrameRate
 * Signature: (JD)Z
 */
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_setFrameRate(
    JNIEnv *env, jclass, jlong handle, jdouble frameRate) {
  try {
    auto instance = getCameraInstance(handle);
    if (!instance)
      return JNI_FALSE;

    instance->setFrameRate(frameRate);
    return JNI_TRUE;
  } catch (const GenericException &e) {
    std::cerr << "Caught Basler GenericException: " << e.GetDescription()
              << std::endl;
    std::cerr.flush();
  }

  return JNI_FALSE;
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setWhiteBalance
 * Signature: (J[D)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setWhiteBalance(JNIEnv *env, jclass,
                                                        jlong handle,
                                                        jdoubleArray rgb) {
  auto instance = getCameraInstance(handle);
  if (!instance)
    return JNI_FALSE;

  jsize length = env->GetArrayLength(rgb);
  if (length != 3) {
    std::cerr << "Expected array of length 3 for RGB balance, got " << length
              << std::endl;
    return JNI_FALSE; // Expecting an array of length 3
  }

  jdouble buffer[3];
  env->GetDoubleArrayRegion(rgb, 0, 3, buffer);
  try {
    instance->setWhiteBalance({buffer[0], buffer[1], buffer[2]});
    return JNI_TRUE;
  } catch (const GenericException &e) {
    std::cerr << "Caught Basler GenericException in setWhiteBalance: "
              << e.GetDescription() << std::endl;
    return JNI_FALSE;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setAutoWhiteBalance
 * Signature: (JZ)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setAutoWhiteBalance(JNIEnv *env, jclass,
                                                            jlong handle,
                                                            jboolean enable) {
  try {
    auto instance = getCameraInstance(handle);
    if (!instance)
      return JNI_FALSE;

    instance->setAutoWhiteBalance(enable);

    return JNI_TRUE;
  } catch (const GenericException &) {
    return JNI_FALSE;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setPixelFormat
 * Signature: (JI)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_setPixelFormat(JNIEnv *, jclass,
                                                       jlong handle,
                                                       jint format) {
  try {
    auto instance = getCameraInstance(handle);
    if (!instance)
      return JNI_FALSE;

    instance->setPixelFormat(format);
    return JNI_TRUE;
  } catch (const GenericException &e) {
    std::cerr << "Caught Basler GenericException in setPixelFormat: "
              << e.GetDescription() << std::endl;
  }
  return JNI_FALSE;
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getExposure
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_getExposure(
    JNIEnv *env, jclass, jlong handle) {
  try {
    auto instance = getCameraInstance(handle);
    if (!instance)
      return -1.0;

    return instance->getExposure();
  } catch (const GenericException &) {
    return -1.0;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getGain
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_getGain(
    JNIEnv *env, jclass, jlong handle) {
  try {
    auto instance = getCameraInstance(handle);
    if (!instance)
      return -1.0;

    return instance->getGain();
  } catch (const GenericException &) {
    return -1.0;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getAutoExposure
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getAutoExposure(JNIEnv *env, jclass,
                                                        jlong handle) {
  try {
    auto instance = getCameraInstance(handle);
    if (!instance)
      return JNI_FALSE;

    return instance->getAutoExposure() ? JNI_TRUE : JNI_FALSE;
  } catch (const GenericException &) {
    return JNI_FALSE;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getFrameRate
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_getFrameRate(
    JNIEnv *env, jclass, jlong handle) {
  try {
    auto instance = getCameraInstance(handle);
    if (!instance)
      return -1.0;

    return instance->getFrameRate();
  } catch (const GenericException &) {
    return -1.0;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getWhiteBalance
 * Signature: (J)[D
 */
JNIEXPORT jdoubleArray JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getWhiteBalance(JNIEnv *env, jclass,
                                                        jlong handle) {
  try {
    auto instance = getCameraInstance(handle);
    if (!instance)
      return nullptr;

    std::array<double, 3> balance = instance->getWhiteBalance();
    jdoubleArray result = env->NewDoubleArray(3);
    if (!result)
      return nullptr;

    env->SetDoubleArrayRegion(result, 0, 3, balance.data());
    return result;
  } catch (const GenericException &) {
    std::cerr << "Caught Basler GenericException in getWhiteBalance"
              << std::endl;
    return nullptr;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getAutoWhiteBalance
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getAutoWhiteBalance(JNIEnv *env, jclass,
                                                            jlong handle) {
  try {
    auto instance = getCameraInstance(handle);
    if (!instance)
      return JNI_FALSE;

    return instance->getAutoWhiteBalance() ? JNI_TRUE : JNI_FALSE;
    return JNI_FALSE;
  } catch (const GenericException &) {
    return JNI_FALSE;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getSupportedPixelFormats
 * Signature: (J)[I
 */
JNIEXPORT jintArray JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getSupportedPixelFormats(JNIEnv *env,
                                                                 jclass,
                                                                 jlong handle) {
  try {
    auto instance = getCameraInstance(handle);
    if (!instance)
      return nullptr;

    std::vector<int> formats = instance->getSupportedPixelFormats();
    jintArray result = env->NewIntArray(formats.size());
    if (!result)
      return nullptr;

    env->SetIntArrayRegion(result, 0, formats.size(), formats.data());
    return result;
  } catch (const GenericException &) {
    return nullptr;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getPixelFormat
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_getPixelFormat(
    JNIEnv *env, jclass, jlong handle) {
  try {
    auto instance = getCameraInstance(handle);
    if (!instance)
      return -1;
    return instance->getPixelFormat();
  } catch (const GenericException &) {
    return -1;
  }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getMinExposure
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getMinExposure(JNIEnv *, jclass,
                                                       jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) {
    return -1;
  }
  return instance->getMinExposure();
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getMaxExposure
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getMaxExposure(JNIEnv *, jclass,
                                                       jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) {
    return -1;
  }
  return instance->getMaxExposure();
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getMinWhiteBalance
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getMinWhiteBalance(JNIEnv *, jclass,
                                                           jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) {
    return -1;
  }
  return instance->getMinWhiteBalance();
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getMaxWhiteBalance
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL
Java_org_teamdeadbolts_basler_BaslerJNI_getMaxWhiteBalance(JNIEnv *, jclass,
                                                           jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance) {
    return -1;
  }
  return instance->getMaxWhiteBalance();
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    takeFrame
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_takeFrame(
    JNIEnv *env, jclass, jlong handle) {

  auto instance = getCameraInstance(handle);
  if (!instance)
    return 0;

  // takeFrame() returns a std::shared_ptr<cv::Mat> copy (your
  // CameraInstance::takeFrame already locks the mutex when copying
  // currentFramePtr).
  auto matPtr = instance->takeFrame();
  if (!matPtr)
    return 0;

  // Defensive checks
  if (matPtr->empty() || matPtr->cols <= 0 || matPtr->rows <= 0) {
    return 0;
  }

  // Allocate a new cv::Mat on the heap that Java will own.
  // Clone ensures the returned Mat's data is independent of any native buffers.
  cv::Mat *javaMat = new cv::Mat(matPtr->clone());
  return reinterpret_cast<jlong>(javaMat);
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    awaitaNewFrame
 * Signature: (J)J
 */
JNIEXPORT void JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_awaitNewFrame(
    JNIEnv *env, jclass, jlong handle) {
  auto instance = getCameraInstance(handle);
  if (!instance)
    return;

  instance->awaitNewFrame();
}

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