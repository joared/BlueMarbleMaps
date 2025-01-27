#ifndef BLUEMARBLE_IMAGEDATAOPERATIONS
#define BLUEMARBLE_IMAGEDATAOPERATIONS

namespace BlueMarble
{

void interleavedToPlanar(unsigned char* planarOut, const unsigned char* interleaved,
                         int width, int height, int channels) {
    int pixelCount = width * height;
    
    for (int c = 0; c < channels; ++c) 
    {
        for (int i = 0; i < pixelCount; ++i) 
        {
            planarOut[c * pixelCount + i] = interleaved[i * channels + c];
        }
    }
}

inline void interleavedToPlanarFlipY(unsigned char* planarOut, const unsigned char* interleaved, 
                                     int width, int height, int channels) {
    int pixelCount = width * height;

    for (int c = 0; c < channels; ++c) // Loop through channels
    { 
        for (int y = 0; y < height; ++y) // Loop through rows
        { 
            int flippedY = height - 1 - y; // Flipped row index
            for (int x = 0; x < width; ++x) // Loop through columns
            { 
                int srcIndex = (flippedY * width + x) * channels + c;
                int dstIndex = c * pixelCount + (y * width + x);
                planarOut[dstIndex] = interleaved[srcIndex];
            }
        }
    }
}

inline void planarToInterleaved(unsigned char* interleavedOut, const unsigned char* planar,
                                int width, int height, int channels) {
    int pixelCount = width * height;

    for (int i = 0; i < pixelCount; ++i) 
    {
        for (int c = 0; c < channels; ++c) 
        {
            interleavedOut[i * channels + c] = planar[c * pixelCount + i];
        }
    }
}


void planarToInterleavedFlipY(unsigned char* interleavedOut, const unsigned char* planar, 
                              int width, int height, int channels) {
    int pixelCount = width * height;

    for (int y = 0; y < height; ++y)           // Loop through rows
    { 
        int flippedY = height - 1 - y;         // Flipped row index
        for (int x = 0; x < width; ++x)        // Loop through columns
        { 
            for (int c = 0; c < channels; ++c) // Loop through channels
            { 
                int srcIndex = c * pixelCount + (flippedY * width + x);
                int dstIndex = (y * width + x) * channels + c;
                interleavedOut[dstIndex] = planar[srcIndex];
            }
        }
    }
}


}

#endif /* BLUEMARBLE_IMAGEDATAOPERATIONS */
