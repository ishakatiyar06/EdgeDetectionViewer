\
    #include <jni.h>
    #include <cstdint>
    #include <vector>
    #include <cmath>
    #include <android/log.h>
    #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "edgeproc", __VA_ARGS__)
    #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "edgeproc", __VA_ARGS__)

    extern "C" JNIEXPORT jbyteArray JNICALL
    Java_com_example_edgeviewer_MainActivity_processFrameRGBA(
            JNIEnv* env,
            jobject /* this */,
            jbyteArray inputArray,
            jint width,
            jint height) {

        jbyte* inputBytes = env->GetByteArrayElements(inputArray, NULL);
        if (inputBytes == NULL) {
            return NULL;
        }

        int w = (int)width;
        int h = (int)height;
        int pixels = w * h;
        // input assumed RGBA (4 bytes per pixel)
        uint8_t* in = reinterpret_cast<uint8_t*>(inputBytes);

        // allocate grayscale
        std::vector<uint8_t> gray(pixels);
        for (int i = 0; i < pixels; ++i) {
            int idx = i * 4;
            uint8_t r = in[idx];
            uint8_t g = in[idx + 1];
            uint8_t b = in[idx + 2];
            // convert to luminance
            uint8_t lum = static_cast<uint8_t>(0.299f * r + 0.587f * g + 0.114f * b);
            gray[i] = lum;
        }

        // sobel kernels
        auto idxAt = [&](int x, int y){ return y * w + x; };

        std::vector<uint8_t> edges(pixels, 0);
        for (int y = 1; y < h - 1; ++y) {
            for (int x = 1; x < w - 1; ++x) {
                int gx = -gray[idxAt(x-1,y-1)] - 2*gray[idxAt(x-1,y)] - gray[idxAt(x-1,y+1)]
                         + gray[idxAt(x+1,y-1)] + 2*gray[idxAt(x+1,y)] + gray[idxAt(x+1,y+1)];
                int gy = -gray[idxAt(x-1,y-1)] - 2*gray[idxAt(x,y-1)] - gray[idxAt(x+1,y-1)]
                         + gray[idxAt(x-1,y+1)] + 2*gray[idxAt(x,y+1)] + gray[idxAt(x+1,y+1)];
                int mag = std::abs(gx) + std::abs(gy);
                if (mag > 150) edges[idxAt(x,y)] = 255;
                else edges[idxAt(x,y)] = 0;
            }
        }

        // allocate output RGBA
        int outSize = pixels * 4;
        std::vector<uint8_t> out(outSize);
        for (int i = 0; i < pixels; ++i) {
            uint8_t e = edges[i];
            int idx = i * 4;
            out[idx] = e;      // R
            out[idx+1] = e;    // G
            out[idx+2] = e;    // B
            out[idx+3] = 255;  // A
        }

        jbyteArray outArray = env->NewByteArray(outSize);
        env->SetByteArrayRegion(outArray, 0, outSize, reinterpret_cast<jbyte*>(out.data()));

        env->ReleaseByteArrayElements(inputArray, inputBytes, JNI_ABORT);
        return outArray;
    }
