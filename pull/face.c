#include "arcsoft_face_sdk.h"
#include "amcomdef.h"
#include "asvloffscreen.h"
#include "merror.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define NSCALE 16 
#define FACENUM	5

#define SafeFree(p) { if ((p)) free(p); (p) = NULL; }

//convert image
int ColorSpaceConversion(MInt32 width, MInt32 height, MInt32 format, MUInt8* imgData, ASVLOFFSCREEN offscreen)
{
	offscreen.u32PixelArrayFormat = (unsigned int)format;
	offscreen.i32Width = width;
	offscreen.i32Height = height;
	
	switch (offscreen.u32PixelArrayFormat)
	{
		case ASVL_PAF_RGB24_B8G8R8:
			offscreen.pi32Pitch[0] = offscreen.i32Width * 3;
			offscreen.ppu8Plane[0] = imgData;
			break;
		case ASVL_PAF_I420:
			offscreen.pi32Pitch[0] = width;
			offscreen.pi32Pitch[1] = width >> 1;
			offscreen.pi32Pitch[2] = width >> 1;
			offscreen.ppu8Plane[0] = imgData;
			offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.i32Height*offscreen.i32Width;
			offscreen.ppu8Plane[2] = offscreen.ppu8Plane[0] + offscreen.i32Height*offscreen.i32Width * 5 / 4;
			break;
		case ASVL_PAF_NV12:

		case ASVL_PAF_NV21:
			offscreen.pi32Pitch[0] = offscreen.i32Width;
			offscreen.pi32Pitch[1] = offscreen.pi32Pitch[0];
			offscreen.ppu8Plane[0] = imgData;
			offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.pi32Pitch[0] * offscreen.i32Height;
			break;

		case ASVL_PAF_YUYV:
		case ASVL_PAF_DEPTH_U16:
			offscreen.pi32Pitch[0] = offscreen.i32Width * 2;
			offscreen.ppu8Plane[0] = imgData;
			break;
		case ASVL_PAF_GRAY:
			offscreen.pi32Pitch[0] = offscreen.i32Width;
			offscreen.ppu8Plane[0] = imgData;
			break;
		default:
			return 0;
	}
	return 1;
}


/*from which image to get which feature of persion*/

int face_feature_detection(char *imgfile, char *face_mesg)
{
	MRESULT res = MOK;
	MHandle handle = NULL;
	char appid[64] =  "asdfkuhaksdhfasF3qc72g6Gasdfasdfay8ap5zxwjBvHdtB";
	char sdkkey[64] =  "asdlfasdklfjlas9rrpjigwg3ks1uVsdfaZNfAdZCC";
	int Width = 640;
	int Height = 480;
	int format = ASVL_PAF_NV21;
	MUInt8* imageData = (MUInt8*)malloc(Height*Width*3/2);
	FILE* fp = fopen(imgfile, "rb");
	MInt32 mask;
	MInt32 processMask;
	int age;
	char gender[8] = "";

	res = ASFOnlineActivation(appid,sdkkey);
	if (MOK != res && MERR_ASF_ALREADY_ACTIVATED != res)
		printf("ASFOnlineActivation fail: %d\n", (int)res);

	//init the engine and filter 
	mask = ASF_FACE_DETECT | ASF_FACERECOGNITION | ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE | ASF_LIVENESS | ASF_IR_LIVENESS;
	res = ASFInitEngine(ASF_DETECT_MODE_IMAGE, ASF_OP_0_ONLY, NSCALE, FACENUM, mask, &handle);
	if (res != MOK)
		printf("ASFInitEngine fail: %d\n", (int)res);
	
	if (fp != NULL)
	{
		fread(imageData, 1, Height*Width*3/2, fp);	
		fclose(fp);

		ASVLOFFSCREEN offscreen = { 0 };
		ColorSpaceConversion(Width,Height,format,imageData,offscreen);
		
		ASF_MultiFaceInfo detectedFaces = { 0 };

		/*detected faces created*/	
		res = ASFDetectFacesEx(handle, &offscreen, &detectedFaces);
		if (res != MOK && detectedFaces.faceNum > 0)
			printf("ASFDetectFacesEx fail: %d\n",(int)res);

		printf("\n************* Face Process *****************\n");
		ASF_LivenessThreshold threshold = { 0 };

		threshold.thresholdmodel_BGR = 0.5;
		threshold.thresholdmodel_IR = 0.7;

		res = ASFSetLivenessParam(handle, &threshold);
		if (res != MOK)
			printf("ASFSetLivenessParam fail: %d\n", (int)res);
		printf("RGB Threshold: %f\nIR Threshold: %f\n", threshold.thresholdmodel_BGR, threshold.thresholdmodel_IR);

		processMask = ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE | ASF_LIVENESS;
		res = ASFProcessEx(handle, &offscreen, &detectedFaces, processMask);
		if (res != MOK)
			printf("ASFProcessEx fail: %d\n", (int)res);

		/*features of person*/
		ASF_AgeInfo ageInfo = { 0 };
		ASF_GenderInfo genderInfo = { 0 };
		ASF_Face3DAngle angleInfo = { 0 };
		ASF_LivenessInfo rgbLivenessInfo = { 0 };
		
		res = ASFGetAge(handle, &ageInfo);/*Get age information*/
		if (res != MOK)
			printf("ASFGetAge fail: %d\n",(int)res);
		else
			printf("The age: [%d]\n",ageInfo.ageArray[0]);

		res = ASFGetGender(handle, &genderInfo);/*Get gender information*/
		if (res != MOK)
			printf("ASFGetGender fail: %d\n",(int)res);
		else
			printf("The gender: [%d]\n",genderInfo.genderArray[0]);

		res = ASFGetFace3DAngle(handle, &angleInfo);/*Get face 3D information*/
		if (res != MOK)
			printf("ASFGetFace3DAngle fail: %d\n",(int)res);
		else
			printf("The 3dAngle: roll: [%lf] yaw: [%lf] pitch: [%lf]\n",angleInfo.roll[0], angleInfo.yaw[0], angleInfo.pitch[0]);
		
		res = ASFGetLivenessScore(handle, &rgbLivenessInfo);/*Get live score information*/
		if (res != MOK)
			printf("ASFGetLivenessScore fail: %d\n", (int)res);
		else
			printf("ASFGetLivenessScore sucess: %d\n", rgbLivenessInfo.isLive[0]);
		
		age = ageInfo.ageArray[0];
		switch(genderInfo.genderArray[0])
		{
			case 1:
				strcpy(gender,"male");
				break;
			case 0:
				strcpy(gender,"female");
				break;
			default:
				strcpy(gender,"none");
				break;
		}

		sprintf(face_mesg,"age[%d],gender[%s]",age,gender);
		/*deleted data*/
		memset(imageData,0,Height*Width*3/2);
		
		//uninit engine
		res = ASFUninitEngine(handle);
		if (res != MOK)
			printf("ASFUninitEngine fail: %d\n", (int)res);
	}
	else
		printf("No pictures found.\n");

    return 0;
}

