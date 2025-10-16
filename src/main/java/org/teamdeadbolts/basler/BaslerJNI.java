/* Team Deadbolts (C) 2025 */
package org.teamdeadbolts.basler;

import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.imgproc.Imgproc;

public class BaslerJNI {
    public enum CameraModel {
        Disconnected,
        daA1280_54uc,
        Unknown;

        public String getFriendlyName() {
            switch (this) {
                case Disconnected:
                    return "Disconnected Camera";
                case daA1280_54uc:
                    return "Basler daA1280-54uc";
                case Unknown:
                default:
                    return "Unknown Basler Camera";
            }
        }
    }

    public static class CvConversion {
        public final int cvType;
        public final int cvtCode; // -1 if no conversion needed

        public CvConversion(int cvType, int cvtCode) {
            this.cvType = cvType;
            this.cvtCode = cvtCode;
        }
    }

    public static boolean isSupported() {
        return isLibraryWorking();
    }

    private static native boolean isLibraryWorking();

    /** Get the camera model from a given serial number or device ID. */
    public static CameraModel getCameraModel(String serial) {
        int model = getCameraModelRaw(serial);
        CameraModel[] values = CameraModel.values();
        if (model < 0 || model >= values.length) return CameraModel.Unknown;
        return values[model];
    }

    public static native int getCameraModelRaw(String serial);

    public static native String[] getConnectedCameras();

    /**
     * Create a new camera instance by serial number.
     *
     * @param serial The serial number or user-defined name of the camera.
     * @return Native camera pointer (0 if failed).
     */
    public static native long createCamera(String serial);

    /** Start streaming frames. */
    public static native boolean startCamera(long ptr);

    /** Stop streaming frames. */
    public static native boolean stopCamera(long ptr);

    /** Destroy the camera and release all native resources. */
    public static native boolean destroyCamera(long ptr);

    public static native boolean setExposure(long ptr, double exposureMs);

    public static native boolean setGain(long ptr, double gain);

    public static native boolean setAutoExposure(long ptr, boolean enable);

    public static native boolean setFrameRate(long ptr, double fps);

    public static native boolean setWhiteBalance(long ptr, double temperature);

    public static native boolean setAutoWhiteBalance(long ptr, boolean enable);

    public static native boolean setPixelFormat(long ptr, int format);

    public static native double getExposure(long ptr);

    public static native double getGain(long ptr);

    public static native boolean getAutoExposure(long ptr);

    public static native double getFrameRate(long ptr);

    public static native double getWhiteBalance(long ptr);

    public static native boolean getAutoWhiteBalance(long ptr);

    /** Get pointer to the latest captured frame. */
    public static native long takeFrame(long ptr);

    public static native long awaitNewFrame(long ptr);

    /**
     * Get frame dimensions from a frame buffer pointer.
     *
     * @param cameraPtr Camera handle
     * @param framePtr Frame buffer pointer from takeFrame() or awaitNewFrame()
     * @return int array [width, height] or null if invalid
     */
    public static native int[] getFrameDimensionsFromBuffer(long cameraPtr, long framePtr);

    // /**
    //  * Get pixel format from a frame buffer pointer.
    //  * Returns 0=Mono8, 1=RGB8, 2=BGR8, 3=Mono16, -1=unknown
    //  *
    //  * @param cameraPtr Camera handle
    //  * @param framePtr Frame buffer pointer from takeFrame() or awaitNewFrame()
    //  * @return Pixel format code
    //  */
    // public static native int getFramePixelFormatFromBuffer(long cameraPtr, long framePtr);

    public static native int getPixelFormat(long cameraPtr);

    /**
     * Get raw frame data from a frame buffer pointer.
     *
     * @param cameraPtr Camera handle
     * @param framePtr Frame buffer pointer from takeFrame() or awaitNewFrame()
     * @return Raw image data or null if invalid
     */
    public static native byte[] getFrameDataFromBuffer(long cameraPtr, long framePtr);

    public static native void cleanUp();

    /**
     * Convert a frame buffer to an OpenCV Mat.
     *
     * @param cameraPtr Camera handle
     * @param framePtr Frame buffer pointer from takeFrame() or awaitNewFrame()
     * @return OpenCV Mat containing the frame, or null if failed
     */
    public static Mat frameToMat(long cameraPtr, long framePtr) {
        if (framePtr == 0) {

            return null;
        }

        int[] dims = getFrameDimensionsFromBuffer(cameraPtr, framePtr);
        if (dims == null || dims.length != 2) {
            return null;
        }

        int width = dims[0];
        int height = dims[1];
        BaslerPixelFormat format = BaslerPixelFormat.fromValue(getPixelFormat(cameraPtr));
        byte[] data = getFrameDataFromBuffer(cameraPtr, framePtr);

        if (data == null) {
            return null;
        }

        CvConversion conversion = getCvConversion(format);
        Mat mat = new Mat(height, width, conversion.cvType);
        mat.put(0, 0, data);

        if (conversion.cvtCode != -1) {
            Mat colorMat = new Mat();
            Imgproc.cvtColor(mat, colorMat, conversion.cvtCode);
            mat.release();
            return colorMat;
        }

        return mat;
    }

    public static CvConversion getCvConversion(BaslerPixelFormat format) {
        int cvType;
        int cvtCode = -1;

        switch (format) {
            // Mono 8-bit
            case Mono8:
            case Mono8Signed:
                cvType = CvType.CV_8UC1;
                break;

            // Bayer 8-bit (needs debayering)
            case BayerBG8:
                cvType = CvType.CV_8UC1;
                cvtCode = Imgproc.COLOR_BayerBG2BGR;
                break;
            case BayerGB8:
                cvType = CvType.CV_8UC1;
                cvtCode = Imgproc.COLOR_BayerGB2BGR;
                break;
            case BayerGR8:
                cvType = CvType.CV_8UC1;
                cvtCode = Imgproc.COLOR_BayerGR2BGR;
                break;
            case BayerRG8:
                cvType = CvType.CV_8UC1;
                cvtCode = Imgproc.COLOR_BayerRG2BGR;
                break;

            // Mono 10/12/16-bit
            case Mono10:
            case Mono10p:
            case Mono12:
            case Mono12p:
            case Mono16:
            case BayerBG10:
            case BayerBG10p:
            case BayerGB10:
            case BayerGB10p:
            case BayerGR10:
            case BayerGR10p:
            case BayerRG10:
            case BayerRG10p:
            case BayerBG12:
            case BayerBG12p:
            case BayerGB12:
            case BayerGB12p:
            case BayerGR12:
            case BayerGR12p:
            case BayerRG12:
            case BayerRG12p:
            case BayerBG16:
            case BayerGB16:
            case BayerGR16:
            case BayerRG16:
            case Confidence16:
            case Coord3D_C16:
                cvType = CvType.CV_16UC1;
                break;

            // RGB 8-bit
            case RGB8:
            case RGB8Packed:
                cvType = CvType.CV_8UC3;
                cvtCode = Imgproc.COLOR_RGB2BGR;
                break;

            // BGR 8-bit
            case BGR8:
            case BGR8Packed:
                cvType = CvType.CV_8UC3;
                break;

            // RGBA/BGRA
            case RGBA8Packed:
                cvType = CvType.CV_8UC4;
                cvtCode = Imgproc.COLOR_RGBA2BGR;
                break;
            case BGRA8Packed:
                cvType = CvType.CV_8UC4;
                cvtCode = Imgproc.COLOR_BGRA2BGR;
                break;

            // YUV/YCbCr
            case YCbCr422_8:
            case YUV422_8:
                cvType = CvType.CV_8UC2;
                cvtCode = Imgproc.COLOR_YUV2BGR_YUYV;
                break;
            case YUV422_8_UYVY:
                cvType = CvType.CV_8UC2;
                cvtCode = Imgproc.COLOR_YUV2BGR_UYVY;
                break;
            case YUV422_YUYV_Packed:
            case YUV422Packed:
                cvType = CvType.CV_8UC2;
                cvtCode = Imgproc.COLOR_YUV2BGR_YUYV;
                break;
            case YCbCr420_8_YY_CbCr_Semiplanar:
                cvType = CvType.CV_8UC1;
                cvtCode = Imgproc.COLOR_YUV2BGR_NV12;
                break;

            // 3D/float formats
            case Coord3D_ABC32f:
                cvType = CvType.CV_32FC3;
                break;

            case Confidence8:
                cvType = CvType.CV_8UC1;
                break;

            default:
                throw new IllegalArgumentException("Unsupported pixel format: " + format);
        }

        return new CvConversion(cvType, cvtCode);
    }
}
