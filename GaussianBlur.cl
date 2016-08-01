constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
__kernel void gaussian_blur(
        __read_only image2d_t image,
        __constant float * mask,
        __write_only image2d_t blurredImage,
        __private int maskSize
    ) 
{	
const int2 pos = {get_global_id(0), get_global_id(1)};
 float4 temp;
 float4 outputPixel;
    // Collect neighbor values and multiply with Gaussian for R component
    float sum = 0.0f;
    for(int a = -maskSize; a < maskSize+1; a++) {
        for(int b = -maskSize; b < maskSize+1; b++) {
			temp = convert_float4(read_imagef(image, sampler, pos + (int2)(a,b)));
            sum += mask[a+maskSize+(b+maskSize)*(maskSize*2+1)]*temp.x;
        }
    }
	outputPixel.x = sum;
	    // Collect neighbor values and multiply with Gaussian for G component
    sum = 0.0f;
    for(int a = -maskSize; a < maskSize+1; a++) {
        for(int b = -maskSize; b < maskSize+1; b++) {
			temp = convert_float4(read_imagef(image, sampler, pos + (int2)(a,b)));
            sum += mask[a+maskSize+(b+maskSize)*(maskSize*2+1)]*temp.y;
        }
    }
	outputPixel.y = sum;
	    // Collect neighbor values and multiply with Gaussian for B component
    sum = 0.0f;
    for(int a = -maskSize; a < maskSize+1; a++) {
        for(int b = -maskSize; b < maskSize+1; b++) {
			temp = convert_float4(read_imagef(image, sampler, pos + (int2)(a,b)));
            sum += mask[a+maskSize+(b+maskSize)*(maskSize*2+1)]*temp.z;
        }
    }
	outputPixel.z = sum;
	outputPixel.w = 1.0f;
	write_imagef(blurredImage, pos, outputPixel);
	
}
