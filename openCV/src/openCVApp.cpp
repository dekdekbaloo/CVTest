#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include "CinderOpenCv.h"

#include <vector>
using namespace ci;
using namespace ci::app;

struct Video{
	std::vector<ci::Surface> frame;
	gl::Texture currFrame;
	int curr = 0;
	void insertFrame(ci::Surface surf){
		frame.push_back(surf);
	}
	void update(){
		curr++;
		if (curr >= frame.size())curr = 0;
		currFrame = gl::Texture(frame[curr]);
	}
	void draw(){
		gl::draw(currFrame);
	}
};

class openCVApp : public AppNative {
  public:
	void setup();
	void update();
	void draw();
	virtual void mouseMove(MouseEvent event);
	virtual void keyDown(KeyEvent event);
	CaptureRef mCapture;
	gl::Texture	mTexture;
	gl::Texture mTrack;
	Vec2f mMouseLoc= Vec2f(0,0);

	bool record = false;
	Video buff;
	std::vector<Video> mVid;	
	
};



void openCVApp::setup()
{
	
	try {
		mCapture = Capture::create(1280,720);
		mCapture->start();
	}
	catch (...) {
		console() << "Failed to initialize capture" << std::endl;
	}
	setWindowSize(ci::Vec2i(1920,1080));
	mTexture = gl::Texture(loadImage(loadAsset("dfw.jpg")));
		
}   

void openCVApp::update(){
	cv::Mat input,output,HSV;
	cv::Vec3b color;
	cv::Vec4b color4;
	ci::Surface surf;
	if (mCapture) {
		input = cv::Mat(toOcv(mCapture->getSurface()));
		cv::cvtColor(input, HSV, CV_BGR2HSV);		
		cv::Mat rgb[3],alpha;
		cv::split(input, rgb);//split into 3 channels
		color = HSV.at<cv::Vec3b>(cv::Point(mMouseLoc.x, mMouseLoc.y));
		cv::inRange(HSV, cv::Scalar(23, 40,45), cv::Scalar(97, 165, 215),alpha);//select green colors
		cv::bitwise_not(alpha, alpha);//inverse color
		cv::GaussianBlur(alpha, alpha, cv::Size(9, 9), 3.0);//soften edges
		cv::Mat rgba[4] = { rgb[0], rgb[1], rgb[2], alpha };
		cv::merge(rgba, 4, output);//merge r,g,b,alpha


		if (record){
			ci::app::console() << "recording" << std::endl;
			buff.insertFrame(fromOcv(output));
		}


	}	
	//mTexture = gl::Texture(fromOcv(input));
	surf = fromOcv(output);	
	mTrack = gl::Texture(surf);
	
	for (int i = 0; i < mVid.size(); i++){
		mVid[i].update();
	}
	
	//app::console() << color << std::endl;
	
}

void openCVApp::mouseMove(MouseEvent event){
	mMouseLoc = event.getPos();
}
void openCVApp::keyDown(KeyEvent event){
	if (event.getChar() == ' '){
		app::console() << "yolo" << std::endl;
		mTexture = mTrack;
	}
	else if (event.getChar() == 'r'){
		record = !record;
		if (record == false){
			mVid.push_back(buff);//save vid
			Video t;
			buff = t;//renew buffer
			ci::app::console() << "You have saved " << mVid.size()<<" videos."<< std::endl;
		}
	}
}

void openCVApp::draw()
{
	gl::clear(ci::Color(0,0,0));
	gl::enableAlphaBlending();	
	gl::draw(mTexture);
	for (int i = 0; i < mVid.size(); i++){
		mVid[i].draw();
	}
	gl::draw(mTrack);
	
}

CINDER_APP_NATIVE( openCVApp, RendererGl )
