#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <math.h>

#define DEBUG_TAG "NDK_BionicVision"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG , "natdeb", __VA_ARGS__)

static jint width = 800;
static jint height = 480;
static jint res = 25;	//resolution being downsampled to
jint threshold;
jint binary_threshold;
jint gauss[16][16*16];	//array to hold templates
jint lookupTable[255];	//lookup table
jint kernel[3][3] = {-1,-1,-1,-1,8,-1,-1,-1,-1};	//laplacian kernel
int totalsum;
float totalmean;

jdouble gauss2D(jdouble a, int x, int y, jdouble sigx, jdouble sigy)
{
	return a * exp(-((pow(x,2)/(2*pow(sigx, 2)))+(pow(y, 2)/(2*pow(sigy, 2))))) + 0.05;
}

jint getVal(jint row, jint col)	//method to work with 1D arrays as 2D with width as 800
{
	return width*row+col;
}

jint getVal2(jint row, jint col, jint r)	//method to work with 1D arrays as 2D with custom width
{
	return r*row+col;
}

jint rgb(jint number)	//taken from the Color.rgb source code in Android docs
{
	return (0xFF << 24) | (number << 16) | (number << 8) | number;
}

void fillLookupTable()	//this method fills up the pixel intensity lookup table
{
	int i;
	for(i = 0; i < 255; i++)
	{
		//seperate values according to threshold
		lookupTable[i] = (256/threshold)*(i/(256/threshold));
		if(threshold == 2)
		{
			if(i < 128) { lookupTable[i] = 0; }
			else { lookupTable[i] = 255; }
		}
	}
}

void fillGauss()	//this method creates the gaussian dots template for each grayscale level
{
	int h,i,j;
	for(h = 0; h < 16; h++)
	{
		for(i = -8; i < 8; i++)
		{
			for(j = -8; j < 8; j++)
			{
				gauss[h][getVal2(j+8,i+8,16)] = rgb(h*16*gauss2D(1,i,j,2.2,2.2));
			}
		}
	}
}

JNIEXPORT int JNICALL
Java_nav_bv_CustomView_getAdaptive(JNIEnv * env,jobject this)
{
	if(binary_threshold){ return 1;}
	else{ return 0;}
}

JNIEXPORT void JNICALL
Java_nav_bv_CustomView_initializer(JNIEnv * env,jobject this, jint usr_grayscalelevel)
{
	threshold = usr_grayscalelevel;
	binary_threshold = 0;
	fillGauss();
	fillLookupTable();
}

JNIEXPORT void JNICALL
Java_nav_bv_bionicvision_initializer(JNIEnv * env,jobject this, jint usr_threshold, jint usr_binarythreshold)
{
	threshold = usr_threshold;
	binary_threshold = usr_binarythreshold;
	fillGauss();
	fillLookupTable();
}

JNIEXPORT jintArray JNICALL
Java_nav_bv_CustomView_meanArray(JNIEnv * env,jobject this, jint res_w, jint res_h, jbyteArray frame)
{
	jintArray meanArr;
	meanArr = (*env)->NewIntArray(env, res*res);
	jbyte *frameBuffer = (*env)->GetByteArrayElements(env,frame,0);
	jint fill[res*res];
	jint i,j,k,l;
	jint a = 0;
	jint b = 0;	//variables to use as index for meanArr
	jint mean = 0;
	//totalsum = 0;
	//totalmean = 0;

	for(i=40; i < height-40; i=i+res_h)
	{
		for(j=200; j < width-200; j=j+res_w)	//iterate through the cropped image every (crop_h/res) pixels
		{
			jint sum = 0;
			for(k = i; k < res_h+i; k++)
			{
				for(l = j; l < res_w+j; l++)	//iterating through a block
				{
					sum += 0xFF & frameBuffer[getVal(k,l)];
				}
			}
			mean = (lookupTable[sum/(res_h*res_w)]);	//replaces the mean with an appropriate value from the lookup table
			if(binary_threshold) {mean = sum/(res_h*res_w);}	//required for adaptive thresholding, otherwise mean would be thresholded twice
			fill[getVal2(a,b,res)] = mean;
			a++;

			//to calculate sum of intensities of entire downsampled image
			//totalsum += (sum/(res_h*res_w));
		}
		a=0;
		b++;
	}

	//totalmean = totalsum/(res*res);	//to calculate mean intensity over whole 25x25 image

	(*env)->SetIntArrayRegion(env, meanArr, 0, res*res, fill);
	(*env) ->ReleaseByteArrayElements(env, frame, frameBuffer,0);
	return meanArr;
}
//method runs when adaptive thresholding is active
JNIEXPORT jintArray JNICALL
Java_nav_bv_CustomView_meanArrayAdapt(JNIEnv * env,jobject this, jint res_w, jint res_h, jbyteArray frame)
{
	jintArray meanArr;
	meanArr = (*env)->NewIntArray(env, (res+2)*(res+2));
	jbyte *frameBuffer = (*env)->GetByteArrayElements(env,frame,0);
	jint fill[(res+2)*(res+2)];
	jint fillFinal[res*res];
	jint temp[9];
	int i,j,k,l,m,n;
	int sum2;
	int a = 0;
	int b = 0;	//variables to use as index for meanArr
	jint mean = 0;
	totalsum = 0;
	totalmean = 0;
	float x,y;

	for(i=40-3; i < (height-40)+2; i=i+res_h)	//loop to downsample image
	{
		for(j=200-3; j < (width-200)+2; j=j+res_w)	//iterate through the cropped image every (crop_h/res) pixels
		{
			int sum = 0;
			for(k = i; k < res_h+i; k++)
			{
				for(l = j; l < res_w+j; l++)	//iterating through a block
				{
					sum += 0xFF & frameBuffer[getVal(k,l)];
				}
			}
			mean = sum/(res_h*res_w);
			fill[getVal2(a,b,res+2)] = mean;
			a++;
		}
		a=0;
		b++;
	}

	sum2 = 0;

	for(i = 0; i < 27; i++)	//loop to perform Laplacian edge detection
	{
		for(j = 0; j < 27; j++)
		{
			if(i >= 1 && j >= 1 && i <= 25 && j <= 25)
			{
				for(k = -1; k < 2; k++)
				{
					for(l = -1; l < 2; l++)
					{
						//getting current local patch in image and storing it temporarily
						temp[getVal2(k+1,l+1,3)] = fill[getVal2(i-k,j-l,res+2)];
					}
				}
				for(m = 0; m < 3; m++)
				{
					for(n = 0; n < 3; n++)
					{
						//apply laplacian kernel
						sum2 += kernel[m][n]*temp[getVal2(m,n,3)];
					}
				}

				sum2 = abs(sum2);
				fillFinal[getVal2(i-1,j-1,res)] = sum2;
				sum2 = 0;
			}
		}
	}
	for(i = 0; i < res; i++)	//loop to find adaptive threshold
	{
		for(j = 0; j < res; j++)
		{
			int temp = fillFinal[getVal2(i,j,res)];
			//to calculate sum of intensities of entire downsampled image;
			totalsum += temp;
		}
	}
	totalmean = totalsum/(res*res);	//to calculate mean intensity over whole 25x25 image
	x = 1;//
	y = 5;// offset values for the mean/threshold
	totalmean = (x*totalmean) + y;	//apply offset values

	/*for(i = 0; i < 27; i++)
	{
		for(j = 0; j < 27; j++)
		{
			if(i >= 1 && j >= 1 && i <= 26 && j <= 26)
			{
				fillFinal[getVal2(i-1,j-1,res)] = fill[getVal2(i,j,res+2)];
			}
		}
	}*/
	(*env)->SetIntArrayRegion(env, meanArr, 0, res*res, fillFinal);
	(*env) ->ReleaseByteArrayElements(env, frame, frameBuffer,0);
	return meanArr;
}

JNIEXPORT jintArray JNICALL
Java_nav_bv_CustomView_pixelArray(JNIEnv * env,jobject this, jintArray meanArr, jint res_w, jint res_h)
{
	jint *maBuffer = (*env)->GetIntArrayElements(env,meanArr,0);
	jintArray pixels;
	jint tempArray[res_w];	//needed to store the each line of a gaussuan dot

	int crop_w = res_w * res;	//crop_w = 400, crop_h = 400
	int crop_h = res_h * res;

	pixels = (*env)->NewIntArray(env, crop_w*crop_h);

	int i,j,k,l;

	jint temp = 0;
	for(i = 0; i < crop_h; i=i+res_h)
	{
		for(j = 0; j < crop_w; j=j+res_w)	//double loop to iterate through 400x400, jumping every 16x16 blocks
		{
			if(!binary_threshold)
			{
				temp = maBuffer[getVal2(j/res_h,i/res_w,res)];	//get the mean pixel
			}
			else	//go in here when adaptive thresholding is active
			{
				temp = maBuffer[getVal2(j/res_h,i/res_w,res)];	//get the mean pixel
				//LOGD("temp2 = %d totalmean = %d", temp,totalmean);
				if(temp < totalmean) { temp = 0;}
				else{ temp = 255;}
			}
			for(k = 0; k < res_h; k++)
			{
				for(l = 0; l< res_w; l++)	//double loop to go through each 16x16 block
				{
					tempArray[l] = gauss[temp/16][getVal2(k,l,res_h)];	//stores a line of the appropriate gaussian dot (16 pixels long)
				}
				(*env)->SetIntArrayRegion(env, pixels, getVal2(i+k,j,crop_w), res_h, tempArray);	//memcopy the line onto the final pixel array
			}
		}
	}
	(*env) ->ReleaseIntArrayElements(env, meanArr, maBuffer,0);
	return pixels;
}

