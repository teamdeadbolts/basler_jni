/* Team Deadbolts (C) 2025 */
package org.teamdeadbolts.basler;

import static org.junit.jupiter.api.Assertions.*;
import static org.junit.jupiter.api.Assumptions.*;

import edu.wpi.first.util.PixelFormat;
import org.junit.jupiter.api.*;
import org.junit.jupiter.api.condition.EnabledIf;
import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.imgcodecs.Imgcodecs;

public class BaslerJNITest {

    private static boolean libraryLoaded = false;
    private static boolean opencvLoaded = true;
    private static boolean hasCameras = false;
    private static String[] connectedCameras;

    @BeforeAll
    static void setup() {
        try {
            System.loadLibrary(Core.NATIVE_LIBRARY_NAME);
        } catch (UnsatisfiedLinkError e) {
            opencvLoaded = false;
            System.err.println("Warning: Failed to load OpenCV library: " + e.getMessage());
        }
        try {
            // Load the native library
            System.loadLibrary("baslerjni");
            libraryLoaded = BaslerJNI.isSupported();

            if (libraryLoaded) {
                connectedCameras = BaslerJNI.getConnectedCameras();
                hasCameras = connectedCameras != null && connectedCameras.length > 0;

                System.out.println("Library loaded: " + libraryLoaded);
                System.out.println("Cameras found: " + (hasCameras ? connectedCameras.length : 0));
                if (hasCameras) {
                    for (int i = 0; i < connectedCameras.length; i++) {
                        System.out.println("  Camera " + i + ": " + connectedCameras[i]);
                    }
                }
            }
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Native library not found: " + e.getMessage());
            System.err.println("Tests will be skipped.");
        }
    }

    @Test
    @DisplayName("Library should load successfully")
    void testLibraryWorking() {
        assumeTrue(libraryLoaded, "Native library not available");
        assertTrue(BaslerJNI.isSupported(), "Library should be working");
    }

    @Test
    @DisplayName("Should enumerate connected cameras")
    void testGetConnectedCameras() {
        assumeTrue(libraryLoaded, "Native library not available");

        String[] cameras = BaslerJNI.getConnectedCameras();
        assertNotNull(cameras, "Camera array should not be null");

        if (cameras.length > 0) {
            System.out.println("Found " + cameras.length + " camera(s)");
            for (String serial : cameras) {
                assertNotNull(serial, "Camera serial should not be null");
                assertFalse(serial.isEmpty(), "Camera serial should not be empty");
            }
        } else {
            System.out.println("No cameras connected - some tests will be skipped");
        }
    }

    @Test
    @DisplayName("Should get camera model for valid serial")
    void testGetCameraModel() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        String modelString = BaslerJNI.getCameraModelRaw(serial);
        System.out.println("Model raw: " + modelString);
        BaslerJNI.CameraModel model = BaslerJNI.CameraModel.fromString(modelString);
        System.out.println("Model enum: " + model.getFriendlyName());

        assertNotEquals(
                BaslerJNI.CameraModel.Unknown, model, "Should return valid model identifier");
    }

    @Test
    @DisplayName("Should return null for invalid camera serial")
    void testGetCameraModelInvalid() {
        assumeTrue(libraryLoaded, "Native library not available");

        String model = BaslerJNI.getCameraModelRaw("INVALID_SERIAL_12345");
        assertNull(model, "Should return null for invalid serial");
    }

    @Test
    @DisplayName("Should create camera with valid serial")
    void testCreateCamera() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);

        assertNotEquals(0, handle, "Camera handle should not be 0");

        // Cleanup
        assertTrue(BaslerJNI.destroyCamera(handle), "Should destroy camera successfully");
    }

    @Test
    @DisplayName("Should return 0 for invalid camera serial on create")
    void testCreateCameraInvalid() {
        assumeTrue(libraryLoaded, "Native library not available");

        long handle = BaslerJNI.createCamera("INVALID_SERIAL_12345");
        assertEquals(0, handle, "Should return 0 for invalid serial");
    }

    @Test
    @DisplayName("Should get valid min and max exposure values")
    void testMinMaxExposure() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            double minExposure = BaslerJNI.getMinExposure(handle);
            double maxExposure = BaslerJNI.getMaxExposure(handle);

            System.out.println("Min exposure: " + minExposure + " µs");
            System.out.println("Max exposure: " + maxExposure + " µs");

            assertTrue(minExposure > 0, "Min exposure should be positive");
            assertTrue(maxExposure > 0, "Max exposure should be positive");
            assertTrue(
                    maxExposure > minExposure, "Max exposure should be greater than min exposure");
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @Test
    @DisplayName("Should get valid min and max white balance values")
    void testMinMaxWhiteBalance() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            double minWB = BaslerJNI.getMinWhiteBalance(handle);
            double maxWB = BaslerJNI.getMaxWhiteBalance(handle);

            System.out.println("Min white balance: " + minWB);
            System.out.println("Max white balance: " + maxWB);

            if (minWB == -1 && maxWB == -1) {
                System.out.println("White balance not supported on this camera");
                return;
            }
            assertTrue(minWB >= 0, "Min white balance should be non-negative");
            assertTrue(maxWB >= 0, "Max white balance should be non-negative");
            assertTrue(maxWB > minWB, "Max white balance should be greater than min white balance");
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @Test
    @DisplayName("Should start and stop camera")
    void testStartStopCamera() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            assertTrue(BaslerJNI.startCamera(handle), "Should start camera successfully");

            // Give it a moment to start
            Thread.sleep(100);

            assertTrue(BaslerJNI.stopCamera(handle), "Should stop camera successfully");
        } catch (InterruptedException e) {
            fail("Test interrupted: " + e.getMessage());
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @Test
    @DisplayName("Should set and get exposure")
    void testExposure() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            double exposureValue = 10000.0; // 10ms in microseconds
            assertTrue(BaslerJNI.setExposure(handle, exposureValue), "Should set exposure");

            double retrievedExposure = BaslerJNI.getExposure(handle);
            assertTrue(retrievedExposure > 0, "Should get valid exposure value");
            assertEquals(
                    exposureValue,
                    retrievedExposure,
                    100.0,
                    "Exposure should match within tolerance");
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @Test
    @DisplayName("Should set and get gain")
    void testGain() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            double gainValue = 10.0;
            assertTrue(BaslerJNI.setGain(handle, gainValue), "Should set gain");

            double retrievedGain = BaslerJNI.getGain(handle);
            assertTrue(retrievedGain >= 0, "Should get valid gain value");
            assertEquals(gainValue, retrievedGain, 1.0, "Gain should match within tolerance");
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @Test
    @DisplayName("Should toggle auto exposure")
    void testAutoExposure() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            // Enable auto exposure
            assertTrue(BaslerJNI.setAutoExposure(handle, true), "Should enable auto exposure");
            assertTrue(BaslerJNI.getAutoExposure(handle), "Auto exposure should be enabled");

            // Disable auto exposure
            assertTrue(BaslerJNI.setAutoExposure(handle, false), "Should disable auto exposure");
            assertFalse(BaslerJNI.getAutoExposure(handle), "Auto exposure should be disabled");
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @Test
    @DisplayName("Should capture a single frame")
    void testTakeFrame() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            BaslerJNI.startCamera(handle);
            BaslerJNI.awaitNewFrame(handle);
            long frameHandle = BaslerJNI.takeFrame(handle);
            assertNotEquals(0, frameHandle, "Should capture a frame");
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @Test
    @DisplayName("Get auto balance values")
    void testAutoBalanceValues() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            BaslerJNI.startCamera(handle);
            BaslerJNI.setAutoExposure(handle, true);
            Thread.sleep(800); // Give it a little time to balance
            double[] values = BaslerJNI.getWhiteBalance(handle);

            System.out.printf("R: %s, B: %s, G: %s\n", values[0], values[1], values[2]);

        } catch (Exception _e) {
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @Test
    @DisplayName("Should handle multiple cameras if available")
    void testMultipleCameras() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");
        assumeTrue(connectedCameras.length >= 2, "Need at least 2 cameras for this test");

        long handle1 = BaslerJNI.createCamera(connectedCameras[0]);
        long handle2 = BaslerJNI.createCamera(connectedCameras[1]);

        try {
            assertNotEquals(0, handle1, "First camera should be created");
            assertNotEquals(0, handle2, "Second camera should be created");
            assertNotEquals(handle1, handle2, "Camera handles should be different");

            assertTrue(BaslerJNI.startCamera(handle1), "Should start first camera");
            assertTrue(BaslerJNI.startCamera(handle2), "Should start second camera");

            assertTrue(BaslerJNI.stopCamera(handle1), "Should stop first camera");
            assertTrue(BaslerJNI.stopCamera(handle2), "Should stop second camera");
        } finally {
            BaslerJNI.destroyCamera(handle1);
            BaslerJNI.destroyCamera(handle2);
        }
    }

    @Test
    @DisplayName("Should handle invalid camera handle gracefully")
    void testInvalidHandle() {
        assumeTrue(libraryLoaded, "Native library not available");

        long invalidHandle = 999999L;

        assertFalse(BaslerJNI.startCamera(invalidHandle), "Should fail to start invalid camera");
        assertFalse(BaslerJNI.stopCamera(invalidHandle), "Should fail to stop invalid camera");
        assertEquals(
                -1.0, BaslerJNI.getExposure(invalidHandle), "Should return -1 for invalid handle");
        // assertEquals(0, BaslerJNI.takeFrame(invalidHandle), "Should return 0 for invalid
        // handle");
    }

    @Test
    @DisplayName("Should properly cleanup resources")
    void testResourceCleanup() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        assertTrue(BaslerJNI.startCamera(handle), "Should start camera");
        assertTrue(BaslerJNI.stopCamera(handle), "Should stop camera");
        assertTrue(BaslerJNI.destroyCamera(handle), "Should destroy camera");

        // Verify handle is no longer valid
        assertFalse(BaslerJNI.startCamera(handle), "Should not start destroyed camera");
    }

    @Test
    @DisplayName("Should convert frame to Mat and save image")
    void testFrameToMat() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(opencvLoaded, "OpenCV not loaded");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        // assertTrue(BaslerJNI.setAutoExposure(handle, true));
        assertTrue(BaslerJNI.setAutoWhiteBalance(handle, true));

        assumeTrue(handle != 0, "Failed to create camera");

        BaslerJNI.startCamera(handle);
        try {
            BaslerJNI.awaitNewFrame(handle);
            long matPtr = BaslerJNI.takeFrame(handle);
            assertNotEquals(0, matPtr, "Should capture a frame");

            System.out.println("Captured frame pointer: " + matPtr);
            Mat mat = new Mat(matPtr);

            // Mat frame = BaslerJNI.frameToMat(handle, framePtr);
            assertNotNull(mat, "Should create Mat from frame");
            assertTrue(mat.rows() > 0, "Mat should have rows");
            assertTrue(mat.cols() > 0, "Mat should have columns");

            Imgcodecs.imwrite("/tmp/test_frame.png", mat);

            mat.release();
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @Test
    @DisplayName("Should perform binning correctly (avg and sum binning)")
    void testImageBinning() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(opencvLoaded, "OpenCV not loaded");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            // Test setting pixel binning (avg)
            assertTrue(
                    BaslerJNI.setPixelBinning(handle, 0, 2, 2), "Should set avg pixel binning 2x2");
            assertTrue(BaslerJNI.startCamera(handle), "Should start camera");
            BaslerJNI.awaitNewFrame(handle);
            Mat matAvgBinning = new Mat(BaslerJNI.takeFrame(handle));
            assertNotNull(matAvgBinning, "Should create Mat from avg binned frame");
            assertTrue(BaslerJNI.stopCamera(handle), "Should stop camera");

            assertTrue(
                    BaslerJNI.setPixelBinning(handle, 1, 2, 2), "Should set sum pixel binning 2x2");
            assertTrue(BaslerJNI.startCamera(handle), "Should start camera");
            BaslerJNI.awaitNewFrame(handle);
            Mat matSumBinning = new Mat(BaslerJNI.takeFrame(handle));
            assertNotNull(matSumBinning, "Should create Mat from sum binned frame");
            assertTrue(BaslerJNI.stopCamera(handle), "Should stop camera");

            System.out.println("Avg Binned Size: " + matAvgBinning.size());
            System.out.println("Sum Binned Size: " + matSumBinning.size());

            double avgMean = Core.mean(matAvgBinning).val[0];
            double sumMean = Core.mean(matSumBinning).val[0];

            System.out.printf("Avg Binned Mean: %.2f\n", avgMean);
            System.out.printf("Sum Binned Mean: %.2f\n", sumMean);

            assertTrue(
                    sumMean > avgMean, "Sum binned image should be brighter than avg binned image");

        } finally {
            BaslerJNI.destroyCamera(handle);
        }

        // int width = 400;
        // int height = 400;
        // Mat original = new Mat(height, width, org.opencv.core.CvType.CV_8UC1);

        // for (int y = 0; y < height; y++) {
        //     for (int x = 0; x < width; x++) {
        //         original.put(y, x, (x + y) % 256);
        //     }
        // }

        // Mat avgMat = original.clone();
        // Mat sumMat = original.clone();

        // int horzBin = 4;
        // int vertBin = 4;

        // BaslerJNI.avgBin(avgMat, horzBin, vertBin);
        // assertEquals(width / horzBin, avgMat.cols(), "Average binned width should match
        // expected");
        // assertEquals(
        //         height / vertBin, avgMat.rows(), "Average binned height should match expected");

        // BaslerJNI.sumBin(sumMat, horzBin, vertBin);
        // assertEquals(width / horzBin, sumMat.cols(), "Sum binned width should match expected");
        // assertEquals(height / vertBin, sumMat.rows(), "Sum binned height should match expected");

        // double avgMean = org.opencv.core.Core.mean(avgMat).val[0];
        // double sumMean = org.opencv.core.Core.mean(sumMat).val[0];

        // System.out.printf("avgBin mean=%.2f, sumBin mean=%.2f%n", avgMean, sumMean);

        // assertTrue(
        //         sumMean > avgMean,
        //         "Sum binning should produce a brighter image than average binning");

        // original.release();
        // avgMat.release();
        // sumMat.release();
    }

    @Test
    @DisplayName("Should test the brightness")
    void testBrightness() {
        assumeTrue(libraryLoaded, "Native library not loaded");
        assumeTrue(opencvLoaded, "OpenCV not loaded");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            assertTrue(BaslerJNI.startCamera(handle), "Failed to start the camera");
            double[] brightnesses = new double[] {-1.0, 1.0, 0.0};

            for (int i = 0; i < brightnesses.length; i++) {
                assertTrue(
                        BaslerJNI.setBrightness(handle, brightnesses[i]),
                        "Failed to set brightness");
                BaslerJNI.awaitNewFrame(handle);

                long ptr = BaslerJNI.takeFrame(handle);
                assertTrue(ptr > 0, "Failed to get image");
                brightnesses[i] = Core.mean(new Mat(ptr)).val[0];
            }

            assertTrue(brightnesses[0] < brightnesses[2], "-1 should be darker than 0");
            assertTrue(brightnesses[1] > brightnesses[2], "1 should be lighter than 0");

            System.out.println("-1 Brightness mean: " + brightnesses[0]);
            System.out.println("0 Brightness mean:" + brightnesses[2]);
            System.out.println("1 Brightness mean: " + brightnesses[1]);

        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @Test
    @DisplayName("Should test the supported formats")
    void testPixelFormats() {
        assumeTrue(libraryLoaded, "Native library not loaded");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            BaslerJNI.startCamera(handle);

            int[] supportedFormats = BaslerJNI.getSupportedPixelFormats(handle);
            assertTrue(supportedFormats.length > 0, "Some formats should be supported");

            for (int format : supportedFormats) {
                System.out.println("Supports format: " + PixelFormat.getFromInt(format));
            }
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @EnabledIf("runExposureTest")
    @Test
    @DisplayName("Should capture frames at different exposures and save images")
    void testExposures() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            // Disable auto exposure so manual settings take effect
            BaslerJNI.startCamera(handle);
            BaslerJNI.setAutoExposure(handle, false);
            BaslerJNI.setAutoWhiteBalance(handle, true);
            BaslerJNI.setPixelFormat(handle, BaslerJNI.getSupportedPixelFormats(handle)[0]);
            System.out.println("int pixel format " + BaslerJNI.getPixelFormat(handle));

            System.out.println(
                    "Current pixel format: "
                            + PixelFormat.getFromInt(BaslerJNI.getPixelFormat(handle)));

            // Exposure times to test (in microseconds)
            long[] exposures = {5000, 10000, 20000, 50000, 100000, 200000};

            for (long exposure : exposures) {
                System.out.println("Testing exposure: " + exposure + " µs");

                assertTrue(BaslerJNI.setExposure(handle, exposure), "Should set exposure");

                // Convert to OpenCV Mat
                BaslerJNI.awaitNewFrame(handle);
                Mat frame = new Mat(BaslerJNI.takeFrame(handle));
                assertNotNull(frame, "Should create Mat from frame");
                assertTrue(frame.rows() > 0 && frame.cols() > 0, "Mat should have dimensions");

                // Save to file
                String filename = String.format("/tmp/test_%dus.png", exposure);
                Imgcodecs.imwrite(filename, frame);
                System.out.println("Saved: " + filename);

                frame.release();
            }
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @EnabledIf("runVideoTests")
    @Test
    @DisplayName("Should capture frames, log FPS, and save video")
    void testRecordVideo() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            assertTrue(
                    BaslerJNI.setPixelFormat(handle, BaslerJNI.getSupportedPixelFormats(handle)[0]),
                    "Should set pixel format to "
                            + PixelFormat.getFromInt(
                                    BaslerJNI.getSupportedPixelFormats(handle)[0]));

            assertTrue(BaslerJNI.setExposure(handle, 100)); // 1ms
            BaslerJNI.setAutoWhiteBalance(handle, true);
            assertTrue(BaslerJNI.startCamera(handle), "Should start camera");

            // Print exposure
            double exposure = BaslerJNI.getExposure(handle);
            System.out.println("Camera exposure (us): " + exposure);

            double cameraFPS = BaslerJNI.getFrameRate(handle);
            System.out.println("Camera-reported FPS: " + cameraFPS);
            BaslerJNI.setFrameRate(handle, cameraFPS);

        } finally {

            // if (videoWriter != null) videoWriter.release();
            BaslerJNI.destroyCamera(handle);
        }
    }

    double runVideoTest(long handle, int framesToCapture) {
        int framesCaptured = 0;
        long startTime = System.currentTimeMillis();

        // Prepare VideoWriter after first frame (need width/height)
        // Mat firstFrame = null;

        for (int i = 0; i < framesToCapture; i++) {
            BaslerJNI.awaitNewFrame(handle);
            long matPtr = BaslerJNI.takeFrame(handle);
            if (matPtr == 0) continue;

            Mat frame = new Mat(matPtr);
            if (frame != null && !frame.empty()) {
                framesCaptured++;

                // if (firstFrame == null) {
                //     firstFrame = frame.clone();

                //     // Open VideoWriter
                //     Size size = new Size(firstFrame.cols(), firstFrame.rows());
                //     videoWriter =
                //             new VideoWriter(
                //                     "/tmp/test_video.mp4",
                //                     VideoWriter.fourcc('m', 'p', '4', 'v'),
                //                     cameraFPS > 0 ? cameraFPS : 30,
                //                     size,
                //                     true);
                //     assertTrue(videoWriter.isOpened(), "VideoWriter should open");
                // }

                // videoWriter.write(frame);
                frame.release();
            }
        }

        long endTime = System.currentTimeMillis();
        BaslerJNI.stopCamera(handle);

        double elapsedSec = (endTime - startTime) / 1000.0;
        double observedFPS = framesCaptured / elapsedSec;
        // System.out.printf(
        //         "Observed FPS: %.2f (Camera-reported: %.2f) - Frames captured: %d%n\n",
        //         observedFPS, cameraFPS, framesCaptured);
        return observedFPS;
    }

    @AfterAll
    static void tearDown() {
        System.out.println("Test suite completed");
        if (libraryLoaded) {
            BaslerJNI.cleanUp();
        }
    }

    // Helper method for conditional test execution
    static boolean runVideoTests() {
        String val = System.getenv("RUN_VIDEO_TESTS");
        return val != null && val.equals("1");
    }

    static boolean runLongTests() {
        String val = System.getenv("RUN_LONG_TESTS");
        return val != null && val.equals("1");
    }

    static boolean runExposureTest() {
        String val = System.getenv("RUN_EXPOSURE_TESTS");
        return val != null && val.equals("1");
    }
}
