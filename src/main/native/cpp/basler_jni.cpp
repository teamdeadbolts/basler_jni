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
            const char* serialCStr = devices[i].GetSerialNumber();
            if (!serialCStr) continue; // skip invalid devices

            jstring jSerial = env->NewStringUTF(serialCStr);
            if (!jSerial) {
                env->ExceptionClear(); // skip if UTF conversion fails
                continue;
            }

            env->SetObjectArrayElement(result, i, jSerial);
            env->DeleteLocalRef(jSerial);
        }

        return result;
    } catch (const GenericException& e) {
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
    } catch (const GenericException& e) {
        std::cerr << "Caught Basler GenericException: " << e.GetDescription() << std::endl;
        std::cerr.flush();
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
 * Method:    setPixelFormat
 * Signature: (JI)Z
 */
JNIEXPORT jboolean JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_setPixelFormat
  (JNIEnv *, jclass, jlong handle, jint format) {
    try {
      auto instance = getCameraInstance(handle);
      if (!instance) return JNI_FALSE;

      if (instance->camera->PixelFormat.IsWritable()) {
        instance->camera->PixelFormat.SetValue(PixelFormatEnums(format));
        return JNI_TRUE;
      }
    } catch (const GenericException &e) {
      std::cerr << "Caught Basler GenericException in setPixelFormat: " << e.GetDescription() << std::endl;
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
 * Method:    takeFrame
 * Signature: (J)J
 */
// Corrected takeFrame
JNIEXPORT jlong JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_takeFrame
  (JNIEnv* env, jclass, jlong handle) {

    auto instance = getCameraInstance(handle);
    if (!instance) return 0;

    try {
        CGrabResultPtr grabResult;
        if (instance->camera->GrabOne(5000, grabResult, TimeoutHandling_ThrowException)) {
            if (grabResult->GrabSucceeded()) {
                std::lock_guard<std::mutex> lock(instance->frameMutex);
                instance->currentFrame = grabResult;
                return reinterpret_cast<jlong>(grabResult->GetBuffer());
            }
        }
    } catch (const GenericException& e) {
        std::cerr << "Pylon exception in takeFrame: " << e.GetDescription() << std::endl;
    }
    return 0;
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
                // Return pointer to the grab result data
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
 * Method:    getFrameDimensionsFromBuffer
 * Signature: (JJ)[I
 */
JNIEXPORT jintArray JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_getFrameDimensionsFromBuffer
  (JNIEnv* env, jclass, jlong handle, jlong framePtr) {

    auto instance = getCameraInstance(handle);
    if (!instance || framePtr == 0) return nullptr;

    std::lock_guard<std::mutex> lock(instance->frameMutex);
    if (!instance->currentFrame || !instance->currentFrame->GrabSucceeded()) return nullptr;

    jintArray result = env->NewIntArray(2);
    if (!result) return nullptr;

    jint dims[2] = {
        static_cast<jint>(instance->currentFrame->GetWidth()),
        static_cast<jint>(instance->currentFrame->GetHeight())
    };

    env->SetIntArrayRegion(result, 0, 2, dims);
    return result;
}

/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getPixelFormat
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_getPixelFormat
  (JNIEnv *, jclass, jlong handle) {
    
    auto instance = getCameraInstance(handle);
    if (!instance) return 0;

    try {
        auto pixelType = instance->camera->PixelFormat.GetValue();
        PixelFormat_RGB8;
        return static_cast<jint>(pixelType);
    } catch (const GenericException&) {
        return 0;
    }
  }



/*
 * Class:     org_teamdeadbolts_basler_BaslerJNI
 * Method:    getFrameDataFromBuffer
 * Signature: (JJ)[B
 */
JNIEXPORT jbyteArray JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_getFrameDataFromBuffer
  (JNIEnv* env, jclass, jlong handle, jlong framePtr) {

    auto instance = getCameraInstance(handle);
    if (!instance || framePtr == 0) return nullptr;

    std::lock_guard<std::mutex> lock(instance->frameMutex);
    if (!instance->currentFrame || !instance->currentFrame->GrabSucceeded()) return nullptr;

    size_t dataSize = instance->currentFrame->GetImageSize();
    const uint8_t* buffer = static_cast<const uint8_t*>(instance->currentFrame->GetBuffer()); 

    jbyteArray result = env->NewByteArray(dataSize);
    if (!result) return nullptr;

    env->SetByteArrayRegion(result, 0, dataSize, reinterpret_cast<const jbyte*>(buffer));
    return result;
}


JNIEXPORT void JNICALL Java_org_teamdeadbolts_basler_BaslerJNI_cleanUp
  (JNIEnv *, jclass) {
    {
        std::lock_guard<std::mutex> lock(mapMutex);
        cMap.clear(); 
    }
    if (pylonInit) {
      PylonTerminate();
      pylonInit = false;
    }
  }