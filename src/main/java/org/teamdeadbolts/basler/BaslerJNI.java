/* Team Deadbolts (C) 2025 */
package org.teamdeadbolts.basler;

import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.imgproc.Imgproc;

import edu.wpi.first.util.PixelFormat;

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
    
    /**
     * Get pixel format from a frame buffer pointer.
     * Returns 0=Mono8, 1=RGB8, 2=BGR8, 3=Mono16, -1=unknown
     * 
     * @param cameraPtr Camera handle
     * @param framePtr Frame buffer pointer from takeFrame() or awaitNewFrame()
     * @return Pixel format code
     */
    public static native int getFramePixelFormatFromBuffer(long cameraPtr, long framePtr);
    
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
        PixelFormat format = PixelFormat.getFromInt(getFramePixelFormatFromBuffer(cameraPtr, framePtr));
        byte[] data = getFrameDataFromBuffer(cameraPtr, framePtr);
        
        if (data == null) {
            return null;
        }

        System.out.println(format);
        
        // Determine OpenCV type based on pixel format
        int cvType;
        int cvtCode = -1;
        switch (format) {
            case kGray:
                cvType = CvType.CV_8UC1;
                break;
            case kY16:
                cvType = CvType.CV_16UC1;
                break;
            case kBGR:
                cvType = CvType.CV_8UC3;
                break;
            case kYUYV:
                cvType = CvType.CV_8UC2;
                cvtCode = Imgproc.COLOR_YUV2BGR_YUYV;
                break;
            case kUYVY:
                cvType = CvType.CV_8UC2;
                cvtCode = Imgproc.COLOR_YUV2BGR_UYVY;
                break;
            default:
                return null;
        }

        
        // Create Mat and copy data
        Mat mat = new Mat(height, width, cvType);
        mat.put(0, 0, data);

        if (cvtCode != -1) {
            Mat colorMat = new Mat();
            Imgproc.cvtColor(mat, colorMat, cvtCode);
            mat.release();
            return colorMat;
        }
        
        return mat;
    }

}
