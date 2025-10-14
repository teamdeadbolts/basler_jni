#include "org_teamdeadbolts_basler_BaslerJNI.h"
#include <pylon/PylonIncludes.h>
#include <pylon/BaslerUniversalInstantCamera.h>
#include <mutex>
#include <map>

using namespace Pylon;
using namespace Basler_UniversalCameraParams;

struct CameraInstance {
  std::unique_ptr<CBaslerUniversalInstantCamera> camera;
  CGrabResultPtr currentFrame;
  std::mutex frameMutex;

  CameraInstance(IPylonDevice *device) : camera(new CBaslerUniversalInstantCamera(device)) {}
};

static std::map<jlong, std::shared_ptr<CameraInstance>> cMap;
static std::mutex mapMutex;
static bool pylonInit = false;

std::string jstringToString(JNIEnv* env, jstring jStr) {
    if (!jStr) return "";
    const char* chars = env->GetStringUTFChars(jStr, nullptr);
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
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_isLibraryWorking
  (JNIEnv* env, jclass) {
    try {
        if (!pylonInit) {
            PylonInitialize();
            pylonInit = true;
        }
        return JNI_TRUE;
    } catch (const GenericException&) {
        return JNI_FALSE;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getCameraModelRaw
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_getCameraModelRaw
  (JNIEnv* env, jclass, jstring serialNumber) {
    try {
        if (!pylonInit) {
            PylonInitialize();
            pylonInit = true;
        }
        
        std::string serial = jstringToString(env, serialNumber);
        CTlFactory& tlFactory = CTlFactory::GetInstance();
        DeviceInfoList_t devices;
        
        if (tlFactory.EnumerateDevices(devices) > 0) {
            for (const auto& devInfo : devices) {
                if (std::string(devInfo.GetSerialNumber()) == serial) {
                    std::string modelName(devInfo.GetModelName());
                    // Return a hash of the model name as an identifier
                    return static_cast<jint>(std::hash<std::string>{}(modelName));
                }
            }
        }
        return -1;
    } catch (const GenericException&) {
        return -1;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getConnectedCameras
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_getConnectedCameras
  (JNIEnv* env, jclass) {
    try {
        if (!pylonInit) {
            PylonInitialize();
            pylonInit = true;
        }
        
        CTlFactory& tlFactory = CTlFactory::GetInstance();
        DeviceInfoList_t devices;
        size_t numDevices = tlFactory.EnumerateDevices(devices);
        
        jclass stringClass = env->FindClass("java/lang/String");
        jobjectArray result = env->NewObjectArray(numDevices, stringClass, nullptr);
        
        for (size_t i = 0; i < numDevices; i++) {
            std::string serial(devices[i].GetSerialNumber());
            jstring jSerial = env->NewStringUTF(serial.c_str());
            env->SetObjectArrayElement(result, i, jSerial);
            env->DeleteLocalRef(jSerial);
        }
        
        return result;
    } catch (const GenericException&) {
        return env->NewObjectArray(0, env->FindClass("java/lang/String"), nullptr);
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    createCamera
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_createCamera
  (JNIEnv* env, jclass, jstring serialNumber) {
    try {
        if (!pylonInit) {
            PylonInitialize();
            pylonInit = true;
        }
        
        std::string serial = jstringToString(env, serialNumber);
        CTlFactory& tlFactory = CTlFactory::GetInstance();
        
        CDeviceInfo devInfo;
        devInfo.SetSerialNumber(serial.c_str());
        
        IPylonDevice* device = tlFactory.CreateDevice(devInfo);
        auto instance = std::make_shared<CameraInstance>(device);
        instance->camera->Open();
        
        jlong handle = reinterpret_cast<jlong>(instance.get());
        
        {
            std::lock_guard<std::mutex> lock(mapMutex);
            cMap[handle] = instance;
        }
        
        return handle;
    } catch (const GenericException&) {
        return 0;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    startCamera
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_startCamera
  (JNIEnv* env, jclass, jlong handle) {
    try {
        auto instance = getCameraInstance(handle);
        if (!instance) return JNI_FALSE;
        
        instance->camera->StartGrabbing(GrabStrategy_LatestImageOnly);
        return JNI_TRUE;
    } catch (const GenericException&) {
        return JNI_FALSE;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    stopCamera
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_stopCamera
  (JNIEnv* env, jclass, jlong handle) {
    try {
        auto instance = getCameraInstance(handle);
        if (!instance) return JNI_FALSE;
        
        instance->camera->StopGrabbing();
        return JNI_TRUE;
    } catch (const GenericException&) {
        return JNI_FALSE;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    destroyCamera
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_destroyCamera
  (JNIEnv* env, jclass, jlong handle) {
    try {
        auto instance = getCameraInstance(handle);
        if (!instance) return JNI_FALSE;
        
        if (instance->camera->IsGrabbing()) {
            instance->camera->StopGrabbing();
        }
        
        instance->camera->Close();
        
        {
            std::lock_guard<std::mutex> lock(mapMutex);
            cMap.erase(handle);
        }
        
        return JNI_TRUE;
    } catch (const GenericException&) {
        return JNI_FALSE;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setExposure
 * Signature: (JD)Z
 */
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_setExposure
  (JNIEnv* env, jclass, jlong handle, jdouble exposure) {
    try {
        auto instance = getCameraInstance(handle);
        if (!instance) return JNI_FALSE;
        
        instance->camera->ExposureTime.SetValue(exposure);
        return JNI_TRUE;
    } catch (const GenericException&) {
        return JNI_FALSE;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setGain
 * Signature: (JD)Z
 */
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_setGain
  (JNIEnv* env, jclass, jlong handle, jdouble gain) {
    try {
        auto instance = getCameraInstance(handle);
        if (!instance) return JNI_FALSE;
        
        instance->camera->Gain.SetValue(gain);
        return JNI_TRUE;
    } catch (const GenericException&) {
        return JNI_FALSE;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setAutoExposure
 * Signature: (JZ)Z
 */
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_setAutoExposure
  (JNIEnv* env, jclass, jlong handle, jboolean enable) {
    try {
        auto instance = getCameraInstance(handle);
        if (!instance) return JNI_FALSE;
        
        if (enable) {
            instance->camera->ExposureAuto.SetValue(ExposureAuto_Continuous);
        } else {
            instance->camera->ExposureAuto.SetValue(ExposureAuto_Off);
        }
        return JNI_TRUE;
    } catch (const GenericException&) {
        return JNI_FALSE;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setFrameRate
 * Signature: (JD)Z
 */
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_setFrameRate
  (JNIEnv* env, jclass, jlong handle, jdouble frameRate) {
    try {
        auto instance = getCameraInstance(handle);
        if (!instance) return JNI_FALSE;
        
        instance->camera->AcquisitionFrameRateEnable.SetValue(true);
        instance->camera->AcquisitionFrameRate.SetValue(frameRate);
        return JNI_TRUE;
    } catch (const GenericException&) {
        return JNI_FALSE;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setWhiteBalance
 * Signature: (JD)Z
 */
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_setWhiteBalance
  (JNIEnv* env, jclass, jlong handle, jdouble balance) {
    try {
        auto instance = getCameraInstance(handle);
        if (!instance) return JNI_FALSE;
        
        if (instance->camera->BalanceRatioSelector.IsWritable()) {
            instance->camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Red);
            instance->camera->BalanceRatio.SetValue(balance);
        }
        return JNI_TRUE;
    } catch (const GenericException&) {
        return JNI_FALSE;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    setAutoWhiteBalance
 * Signature: (JZ)Z
 */
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_setAutoWhiteBalance
  (JNIEnv* env, jclass, jlong handle, jboolean enable) {
    try {
        auto instance = getCameraInstance(handle);
        if (!instance) return JNI_FALSE;
        
        if (instance->camera->BalanceWhiteAuto.IsWritable()) {
            if (enable) {
                instance->camera->BalanceWhiteAuto.SetValue(BalanceWhiteAuto_Continuous);
            } else {
                instance->camera->BalanceWhiteAuto.SetValue(BalanceWhiteAuto_Off);
            }
        }
        return JNI_TRUE;
    } catch (const GenericException&) {
        return JNI_FALSE;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getExposure
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_getExposure
  (JNIEnv* env, jclass, jlong handle) {
    try {
        auto instance = getCameraInstance(handle);
        if (!instance) return -1.0;
        
        return instance->camera->ExposureTime.GetValue();
    } catch (const GenericException&) {
        return -1.0;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getGain
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_getGain
  (JNIEnv* env, jclass, jlong handle) {
    try {
        auto instance = getCameraInstance(handle);
        if (!instance) return -1.0;
        
        return instance->camera->Gain.GetValue();
    } catch (const GenericException&) {
        return -1.0;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getAutoExposure
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_getAutoExposure
  (JNIEnv* env, jclass, jlong handle) {
    try {
        auto instance = getCameraInstance(handle);
        if (!instance) return JNI_FALSE;
        
        return instance->camera->ExposureAuto.GetValue() != ExposureAuto_Off;
    } catch (const GenericException&) {
        return JNI_FALSE;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getFrameRate
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_getFrameRate
  (JNIEnv* env, jclass, jlong handle) {
    try {
        auto instance = getCameraInstance(handle);
        if (!instance) return -1.0;
        
        if (instance->camera->AcquisitionFrameRate.IsReadable()) {
            return instance->camera->AcquisitionFrameRate.GetValue();
        }
        return -1.0;
    } catch (const GenericException&) {
        return -1.0;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getWhiteBalance
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_getWhiteBalance
  (JNIEnv* env, jclass, jlong handle) {
    try {
        auto instance = getCameraInstance(handle);
        if (!instance) return -1.0;
        
        if (instance->camera->BalanceRatio.IsReadable()) {
            instance->camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Red);
            return instance->camera->BalanceRatio.GetValue();
        }
        return -1.0;
    } catch (const GenericException&) {
        return -1.0;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getAutoWhiteBalance
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_getAutoWhiteBalance
  (JNIEnv* env, jclass, jlong handle) {
    try {
        auto instance = getCameraInstance(handle);
        if (!instance) return JNI_FALSE;
        
        if (instance->camera->BalanceWhiteAuto.IsReadable()) {
            return instance->camera->BalanceWhiteAuto.GetValue() != BalanceWhiteAuto_Off;
        }
        return JNI_FALSE;
    } catch (const GenericException&) {
        return JNI_FALSE;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    awaitNewFrame
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_awaitNewFrame
  (JNIEnv* env, jclass, jlong handle) {
    try {
        auto instance = getCameraInstance(handle);
        if (!instance) return 0;
        
        CGrabResultPtr ptrGrabResult;
        if (instance->camera->RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException)) {
            if (ptrGrabResult->GrabSucceeded()) {
                std::lock_guard<std::mutex> lock(instance->frameMutex);
                instance->currentFrame = ptrGrabResult;
                return reinterpret_cast<jlong>(ptrGrabResult->GetBuffer());
            }
        }
        return 0;
    } catch (const GenericException&) {
        return 0;
    }
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    takeFrame
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_takeFrame
  (JNIEnv* env, jclass, jlong handle) {
    try {
        auto instance = getCameraInstance(handle);
        if (!instance) return 0;
        
        CGrabResultPtr ptrGrabResult;
        if (instance->camera->GrabOne(5000, ptrGrabResult, TimeoutHandling_ThrowException)) {
            if (ptrGrabResult->GrabSucceeded()) {
                std::lock_guard<std::mutex> lock(instance->frameMutex);
                instance->currentFrame = ptrGrabResult;
                return reinterpret_cast<jlong>(ptrGrabResult->GetBuffer());
            }
        }
        return 0;
    } catch (const GenericException&) {
        return 0;
    }
}