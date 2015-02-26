
#include <iostream>
#include <fstream>
#include "WFSTrainer.h"
#include "WFSDetector.h"
#include <time.h>
#include <cstdio>

#include <fstream>
#include <iostream>
#include <cstdio>

#include <iomanip>

#include <windows.h>
#include <string.h>

#include <string>
#include <vector>

//bool sample_normalized = true;

using namespace std;
using namespace cv;

int detect(bool webcam);
void train(TrainInfo train_info);


void PrintTreeStructure() {
	const int nMC = 3;
	const int nB = 29;
	const int nP = 18;

	const int ts[][nMC + 1] = {
		{  1,  2,  3, -1 },  // 0
		{  4,  5, -1, -1 },  // 01
		{ 14, 15, 16, -1 },  // 02
		{  6,  7, -1, -1 },  // 03
		{  8,  9, 10, -1 },  // 04
		{ 11, 12, 13, -1 },  // 05
		{ 17, 18, 19, -1 },  // 06
		{ 20, 21, 22, -1 },  // 07
		{ -1,  4, -1, -1 },  // 07
		{ -1,  9, -1, -1 },  // 08
		{ -1, 14, -1, -1 },  // 10
		{ -1,  5, -1, -1 },  // 11
		{ 23, 24, -1, -1 },  // 12
		{ -1, 15, -1, -1 },  // 13
		{ -1,  6, -1, -1 },  // 14
		{ 25, 26, -1, -1 },  // 15
		{ -1, 16, -1, -1 },  // 16
		{ -1,  7, -1, -1 },  // 17
		{ 27, 28, -1, -1 },  // 18
		{ -1, 17, -1, -1 },  // 19
		{ -1,  8, -1, -1 },  // 20
		{ -1, 13, -1, -1 },  // 21
		{ -1, 18, -1, -1 },  // 22
		{ -1, 10, -1, -1 },  // 23
		{ -1,  1, -1, -1 },  // 24
		{ -1, 11, -1, -1 },  // 25
		{ -1,  2, -1, -1 },  // 26
		{ -1, 12, -1, -1 },  // 27
		{ -1,  3, -1, -1 },  // 28
	};

	int parent[nB];
	int nc[nB];
	int act_pose[nB];
	int leaf_node[nP+1];

	for(int i=0; i<nB; i++) {
		parent[i] = 0;
		nc[i] = 0;
		act_pose[i]=0;
	}

	for(int i=0; i<nB; i++) {
		int c = 0;
		if (ts[i][0] == -1) {
			act_pose[i] = ts[i][1];
			leaf_node[ts[i][1]] = i;
		} else {
			for(int j=0; j<nMC; j++) {
				if (ts[i][j] == -1) break;
				c++;
				parent[ts[i][j]] = i;
			}
		}
		nc[i] = c;
	}

	parent[0] = -1;

	int tt[nB][nP+1];
	int tt2[nB][nP+1];
	for(int i=0; i<nB; i++) {
		int t[nP + 1];
		for(int j=0; j<=nP; j++) t[j] = 0;
		if (ts[i][0] == -1) {
			t[0] = ts[i][1];
		} else {
			for(int j=1; j<=nP; j++) {
				int n = leaf_node[j];
				while(parent[n] >= 0) {
					if (parent[n] == i) {
						t[j] = n;
						break;
					}
					n = parent[n];
				}
			}
		}
		for(int j=0; j<=nP; j++) tt[i][j] = t[j];

		int q[nP + 1];
		q[0] = -1;
		for(int j=0; j<nc[i]; j++) {
			q[ts[i][j]] = j;
		}

		t[0]= 0;
		for(int j=1; j<=nP; j++) 
		{ 
			t[j] = q[t[j]];
		}
		for(int j=0; j<=nP; j++) tt2[i][j] = t[j];

	}

//	WfsTreeCategoryId



	printf("#define K_WFS_POSES %d\n", nP);
	printf("#define K_WFS_BRANCH_NODES %d\n", nB);
	printf("#define K_WFS_MAX_BRANCHES %d\n", nMC);
	printf("\n");

	printf("// parent, # children, ~~~\n");
	printf("const int WfsTreeInfo[][%d] = {\n", nB + 3);
	for(int i=0; i<nB; i++) {
		printf("\t{ %2d, %2d, ", parent[i], nc[i]);
		for(int j=0; j<nc[i]; j++) {
			printf("%2d, ", ts[i][j]);
		}
		printf("}, \n");
	}
	printf("};\n");

	printf("\n");
	printf("const int WfsTreeStructure[][K_WFS_POSES+1] = {\n");
	printf("\t// AP  1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18\n");
	for(int i=0; i<nB; i++) {
		printf("\t{ ");
		for(int j=0; j<=nP; j++) {
			printf("%2d, ", tt[i][j]);
		}
		printf("},   // %2d\n", i);
	}
	printf("};\n");

	printf("\n");
	printf("const int WfsTreeCategoryId[][K_WFS_POSES+1] = {\n");
	printf("\t// AP  1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18\n");
	for(int i=0; i<nB; i++) {
		printf("\t{ ");
		for(int j=0; j<=nP; j++) {
			printf("%2d, ", tt2[i][j]);
		}
		printf("},   // %2d\n", i);
	}
	printf("};\n");

	exit(0);
}


void CreatePosList(string &filename, string &path) 
{

	vector<string> files;

	SearchAllFiles(path, files);

	int n = (int)files.size();

	ofstream list;
	list.open(filename.c_str());

	vector<int> r_list;
	vector<bool> r_exist;

	r_exist.resize(n, false);

	for(int i=0; i<n; i++) {

		int r = 0;
		while (r_exist[r]) 
		{
			r = (rand() * rand()) % n;
		}

		r_list.push_back(r);
		r_exist[r] = true;
	}

	vector<int> n_pose;
	n_pose.resize(K_WFS_POSES+1, 0);

	for(int i=0; i<n; i++) {

		int n = files[r_list[i]].rfind("/");
		string pose_str = files[r_list[i]].substr(n + 7, 2);
		int p = atoi(pose_str.c_str());

		n_pose[p]++;

//		if (p==11) {
			list << files[r_list[i]] << "\n";
			list << p << "\n";
//		}

	}

	cout << "# samples\n";
	for(int i=1; i<=K_WFS_POSES; i++) {
		cout << i << " POSE  : " << n_pose[i] << "\n";
	}
		
	list.close();

}

void CreateNegList(string &filename, string &path) 
{

	vector<string> files;

	SearchAllFiles(path, files);

	int n = (int)files.size();

	ofstream list;
	list.open(filename.c_str());

	for(int i=0; i<n; i++) {
		list << files[i] << "\n";
	}
		
	list.close();

}

void main(){
	
//	vector<GranVec> gv;
//	vector<GranVec> gv2;
//
//	cout << sizeof(GranVec) << "\n";
//
//	//char a,b,c,d,e;
//
//	//a = 255;
//	//b = 255;
//	//c = (a+b) >> 1;
//	//d = -50;
//	//e = (char)((char)a+(char)b+(char)c+(char)d) >> 2;
//
//	GranVec* gva = new GranVec [300000];

//	vector <GranVec*> gv;
//	for(int i=0; i<500000; i++) {
//		GranVec *gg = new GranVec;
//		gv.push_back(gg);
////		gv2.push_back(gg);
//	}
//
//	vector<int> qq;
//	for(int i=0; i<3000000; i++) {
//		qq.push_back(i);
////		gv2.push_back(gg);
//	}
//
//	system("pause");

//	PrintTreeStructure();

	TrainInfo train_info;
	train_info.positive_list_file = "C:/UCLA/MVFD/facedata/posCenter3_3.txt";
	train_info.negative_list_file = "C:/UCLA/MVFD/facedata/neg2_gray.txt";
	//train_info.dir_neg = "C:/UCLA/Nonface16/*.*";
	//train_info.dir_pos = "C:/UCLA/Face16/*.BMP";
	//train_info.dir_neg = "./nonface/*.png";
	//train_info.dir_pos = "./face/*.png";
	//train_info.dir_neg = "./nonface/*.BMP";
	train_info.max_false_alarm = 0.5;
	train_info.min_detection_rate = 0.995;

	train_info.nBinaryStages = 3;

//	train_info.num_stage = 3;
//	train_info.train_num_neg = 4000;
	train_info.train_num_pos = 3000;
	train_info.nNegSamplesRatio = 3.0;
	//train_info.train_num_neg = 1000;
	//train_info.train_num_pos = 500;
	train_info.threshold = 0;

	//C:\UCLA\MVFD\facedata\LTI\0106_2nd
	//CreatePosList(train_info.positive_list_file, string("C:/UCLA/MVFD/facedata/LTI/center3/*.png"));
	//return;

	//CreateNegList(train_info.negative_list_file, string("C:/UCLA/MVFD/facedata/negative/*.jpg"));
	//return;

//	detect(true);
	train(train_info);

	//short a = -1;
	//short b = a << 1;
	//short c = b << 7;

	//cout << a << " " << b << " " << c << " \n";

}
void train(TrainInfo train_info)
{

	int result = 0;
	//sample_normalized = true;

	WfsMvfdTrainer cascade(train_info);
	
	time_t start,end;
	time(&start);

	result = cascade.DoTrain();

//	cascade.ResumeTrain("./output_middle_single_cascade_10_middle_0.xml", 0, 11);
	time(&end);
	cout<< "total training time "<<difftime(end,start)<<" seconds"<<endl;
	cout<< "trained with "<<cascade.getNumPosUsed()<<" positive image  "<<cascade.getNumNegUsed()<<" negative images"<<endl;
	if(result == 0)
		cascade.save("./result/output.xml");

	//cascade.load("./result/output.xml");
	//cascade.checkFeatures();
	
	
}

void DrawPose(IplImage *img, WfsFaceInfo& face) 
{
	int x = face.x;
	int y = face.y;
	int s = face.size;
	int p = face.pose;

	if (p == 11) {
		cvDrawRect(img, cvPoint(x,y), cvPoint(x+s,y+s), cvScalar(0, 255,0), 2);
	}

	if (p == 10) {
		cvDrawLine(img, cvPoint(x,y), cvPoint(x+s,y), cvScalar(255,0,0), 2);
		cvDrawLine(img, cvPoint(x,y+s), cvPoint(x+s,y+s), cvScalar(255,0,0), 2);
		cvDrawLine(img, cvPoint(x+s,y), cvPoint(x+s,y+s), cvScalar(255,0,0), 2);
		cvDrawLine(img, cvPoint(x,y), cvPoint(x-s/3,y+s/2), cvScalar(255,0,0), 2);
		cvDrawLine(img, cvPoint(x,y+s), cvPoint(x-s/3,y+s/2), cvScalar(255,0,0), 2);
	}

	if (p == 12) {
		cvDrawLine(img, cvPoint(x,y), cvPoint(x+s,y), cvScalar(0,0,255), 2);
		cvDrawLine(img, cvPoint(x,y+s), cvPoint(x+s,y+s), cvScalar(0,0,255), 2);
		cvDrawLine(img, cvPoint(x,y), cvPoint(x,y+s), cvScalar(0,0,255), 2);
		cvDrawLine(img, cvPoint(x+s,y), cvPoint(x+s+s/2,y+s/2), cvScalar(0,0,255), 2);
		cvDrawLine(img, cvPoint(x+s,y+s), cvPoint(x+s+s/2,y+s/2), cvScalar(0,0,255), 2);
	}

}

int detect(bool webcam)
{

	WfsMvfdDetector detector("desktop_output_middle_single_cascade_10_middle_1.xml");
//	WfsMvfdDetector detector("./output_middle28.xml");

	if (webcam) {
		CvCapture *capture = 0;
		IplImage  *frame = 0;
		int       key = 0;

		/* initialize camera */
		capture = cvCaptureFromCAM( 0 );

		/* always check */
		if ( !capture ) {
			fprintf( stderr, "Cannot open initialize webcam!\n" );
			return 1;
		}

		/* create a window for the video */
		cvNamedWindow( "result", CV_WINDOW_AUTOSIZE );

		while( key != 'q' ) {
			/* get a frame */
			frame = cvQueryFrame( capture );

			/* always check */
			if( !frame ) break;
		   

			IplImage *img = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, frame->nChannels);
			cvCopyImage(frame, img);

//			if(img == NULL) continue;

			cout << "start detection\n";
			time_t start,end;
			time(&start);
			vector<WfsFaceInfo> faces;
			detector.DetectFace(faces, img, 1.3, 12);
			time(&end);
			cout<< "total detection time "<<difftime(end,start)<<" seconds"<<endl;

			//const static Scalar colors[] =  { CV_RGB(0,0,255),
			//CV_RGB(0,128,255),
			//CV_RGB(0,255,255),
			//CV_RGB(0,255,0),
			//CV_RGB(255,128,0),
			//CV_RGB(255,255,0),
			//CV_RGB(255,0,0),
			//CV_RGB(255,0,255)} ;
			Point center;
			int scale = 1;

			CvFont font;
			double hScale=1.0;
			double vScale=1.0;
			int    lineWidth=1;
			cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);

			for(int j=0; j<faces.size(); j++) {
				DrawPose(img, faces[j]);

				//if (1 || faces[j].pose == 10) {
				//	Point center;
				//	//Scalar color = colors[faces[j].pose];
				//	Scalar color = CV_RGB(255,0,255);
				//	int radius;
				//	radius = faces[j].size/2;
				//	center.x = cvRound((faces[j].x + radius)*scale);
				//	center.y = cvRound((faces[j].y + radius)*scale);
				//	cvCircle( img, center, radius, color, 1, 8, 0 );
				//	
				//	char pose_str[10];
				//	sprintf(pose_str,"%d", faces[j].pose);
				//	cvPutText (img, pose_str,cvPoint(center.x, center.y), &font, cvScalar(255,255,0));
				//}
			}


	//		cvSaveImage((fn+".result_0.jpg").c_str(), img);


			/* display current frame */
			cvShowImage( "result", img );

			static int frame = 0;
			char fn[256];
			sprintf(fn, "./cam/frame%05d.jpg", frame);
			frame++;
			cvSaveImage(fn, img);

			cvReleaseImage( &img);

			/* exit if user press 'q' */
			key = cvWaitKey( 1 );
		}

		/* free memory */
		cvDestroyWindow( "result" );
		cvReleaseCapture( &capture );




	} else {
		vector<string> files;
		string fm = "c:/UCLA/fd_test/*.jpg";
		SearchAllFiles(fm, files);

		int i=0;
		while(i<files.size()) {
			string fn = files[i];
			i++;

			IplImage *img = cvLoadImage(fn.c_str());

			if(img == NULL) continue;

			cout << "start detection\n";
			time_t start,end;
			time(&start);
			vector<WfsFaceInfo> faces;
			detector.DetectFace(faces, img);
			time(&end);
			cout<< "total detection time "<<difftime(end,start)<<" seconds"<<endl;

			//const static Scalar colors[] =  { CV_RGB(0,0,255),
			//CV_RGB(0,128,255),
			//CV_RGB(0,255,255),
			//CV_RGB(0,255,0),
			//CV_RGB(255,128,0),
			//CV_RGB(255,255,0),
			//CV_RGB(255,0,0),
			//CV_RGB(255,0,255)} ;
			Point center;
			int scale = 1;

			CvFont font;
			double hScale=1.0;
			double vScale=1.0;
			int    lineWidth=1;
			cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);

			for(int j=0; j<faces.size(); j++) {
				if (1 || faces[j].pose == 10) {
					Point center;
					//Scalar color = colors[faces[j].pose];
					Scalar color = CV_RGB(255,0,255);
					int radius;
					radius = faces[j].size/2;
					center.x = cvRound((faces[j].x + radius)*scale);
					center.y = cvRound((faces[j].y + radius)*scale);
					cvCircle( img, center, radius, color, 1, 8, 0 );
					
					char pose_str[10];
					sprintf(pose_str,"%d", faces[j].pose);
					cvPutText (img, pose_str,cvPoint(center.x, center.y), &font, cvScalar(255,255,0));
				}
			}

			cv::imshow( "result", img );
			cvWaitKey(3000);

	//		cvSaveImage((fn+".result_0.jpg").c_str(), img);
			cvReleaseImage( &img);

		}
	}

	return 0;
}