// CS 61C Fall 2015 Project 4

// include SSE intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif

// include OpenMP
#if !defined(_MSC_VER)
#include <pthread.h>
#endif
#include <omp.h>

#include "calcDepthOptimized.h"
#include "calcDepthNaive.h"

/* DO NOT CHANGE ANYTHING ABOVE THIS LINE. */


int MAX_THREADS = 8;

float displacementOptimized(int dx, int dy)
{
	float squaredDisplacement = dx * dx + dy * dy;
	float displacement = sqrt(squaredDisplacement);
	return displacement;
}

void calcDepthOptimized (float *depth, float *left, float *right, int imageWidth, int imageHeight, int featureWidth, int featureHeight, int maximumDisplacement)
{
	omp_set_num_threads(MAX_THREADS);
	int blocksizeX = 4;

	/* The two outer for loops iterate through each pixel */  
	#pragma omp parallel for
	for (int y = 0; y < imageHeight; y++)
	{
        int intermediary = y * imageWidth;
		for (int x = 0; x < imageWidth; x++)
		{	
            int counter = intermediary + x;
			/* Set the depth to 0 if looking at edge of the image where a feature box cannot fit. */
			if ((y < featureHeight) || (y >= imageHeight - featureHeight) || (x < featureWidth) || (x >= imageWidth - featureWidth))
			{
				depth[counter] = 0;
				continue;
			}

			float minimumSquaredDifference = -1;
			int minimumDy = 0;
			int minimumDx = 0;

			/* Iterate through all feature boxes that fit inside the maximum displacement box. 
			   centered around the current pixel. */
			for (int dy = -maximumDisplacement; dy <= maximumDisplacement; dy++)
			{
				for (int dx = -maximumDisplacement; dx <= maximumDisplacement; dx++)
				{
					/* Skip feature boxes that dont fit in the displacement box. */
					if (y + dy - featureHeight < 0 || y + dy + featureHeight >= imageHeight || x + dx - featureWidth < 0 || x + dx + featureWidth >= imageWidth)
					{
						continue;
					}

					float squaredDifference = 0;
		        	int boxX;
                    int boxY;
		        	int blockMaxX = featureWidth - blocksizeX;

		        	__m128 difference = _mm_setzero_ps();
		        	__m128 squaredDifferenceSum = _mm_setzero_ps();

					/* Sum the squared difference within a box of +/- featureHeight and +/- featureWidth. */
					for (boxY = -featureHeight; boxY <= featureHeight; boxY += 1)
					{
						for (boxX = -featureWidth; boxX <= blockMaxX; boxX += blocksizeX)
						{
							int leftX = x + boxX;
							int leftY = y + boxY;
							int rightX = x + dx + boxX;
							int rightY = y + dy + boxY;
              
            				__m128 left1 = _mm_loadu_ps((__m128*) (left + leftY * imageWidth + leftX));
            				__m128 right1 = _mm_loadu_ps((__m128*) (right + rightY * imageWidth + rightX));
              
            				difference = _mm_sub_ps(left1, right1);
            				difference = _mm_mul_ps(difference, difference);
            				squaredDifferenceSum = _mm_add_ps(squaredDifferenceSum, difference);
            			}

                        for (; boxX <= featureWidth; boxX++) {
							int leftX = x + boxX;
							int leftY = y + boxY;
              				int rightX = x + dx + boxX;
							int rightY = y + dy + boxY;
              
				            __m128 left1 = _mm_loadu_ps((__m128*) (left + leftY * imageWidth + leftX));
				            __m128 right1 = _mm_loadu_ps((__m128*) (right + rightY * imageWidth + rightX));
				              
				            difference = _mm_sub_ss(left1, right1);
				            difference = _mm_mul_ss(difference, difference);
				            squaredDifferenceSum = _mm_add_ps(squaredDifferenceSum, difference);
                        }
					}
                    
			        squaredDifferenceSum = _mm_hadd_ps(squaredDifferenceSum, squaredDifferenceSum);
			        squaredDifferenceSum = _mm_hadd_ps(squaredDifferenceSum, squaredDifferenceSum);
			        squaredDifference = _mm_cvtss_f32(squaredDifferenceSum);
          
			        // /*
			        // UPDATE THIS PART to avoid store operation. Create new intrinsic to keep values of minimumSquaredDifference,
			        // minimumDx, minimumDy.
			          
			        // Intrinsic 1: Difference Squares.
			        // Intrinsic 2: Dx corresponding with Difference Squares.
			        // Intrinsic 3: Dy corresponding with Difference Squares.
			          
			        // ===
			        // */
			          
			        // float sum_array[4] = {0, 0, 0, 0};
			          
			        // _mm_storeu_ps((__m128*) sum_array, squaredDifferenceSum);
			        // squaredDifference += sum_array[0] + sum_array[1] + sum_array[2] + sum_array[3];

					/* 
					Check if you need to update minimum square difference. 
					This is when either it has not been set yet, the current
					squared displacement is equal to the min and but the new
					displacement is less, or the current squared difference
					is less than the min square difference.
					*/
					if ((minimumSquaredDifference == -1) || (minimumSquaredDifference > squaredDifference) || ((minimumSquaredDifference == squaredDifference) && (dx * dx + dy * dy) < (minimumDx * minimumDx + minimumDy * minimumDy)))
					{
						minimumSquaredDifference = squaredDifference;
						minimumDx = dx;
						minimumDy = dy;
					}
				}
			}

			/* 
			Set the value in the depth map. 
			If max displacement is equal to 0, the depth value is just 0.
			*/
			if (minimumSquaredDifference != -1)
			{
				if (maximumDisplacement == 0)
				{
					depth[counter] = 0;
				}
				else
				{
					depth[counter] = displacementOptimized(minimumDx, minimumDy);
				}
			}
			else
			{
				depth[counter] = 0;
			}
		}
	}
}



