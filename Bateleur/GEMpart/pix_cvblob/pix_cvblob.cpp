////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////


#include <math.h>

#include "pix_cvblob.h"

using namespace cv;

CPPEXTERN_NEW(pix_cvblob);
//CPPEXTERN_NEW_WITH_TWO_ARGS(pix_cvblob, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// pix_cvblob
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_cvblob :: pix_cvblob(/*t_floatarg hi_thresh, t_floatarg lo_thresh*/)
{
    /*inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("mode"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("hi_thresh"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("lo_thresh"));
		
    if (hi_thresh > 0)
	{
		m_hi_thresh = hi_thresh;
	} else {
		m_hi_thresh = 8000.0;
	}
    
    if (lo_thresh > 0)
    {
        m_lo_thresh = lo_thresh;
    } else {
        m_lo_thresh = 10.1;
    }
    
    m_active = true;
	m_mode = true;
	m_gray = false;
	//t_mult = 2048.0 / (m_hi_thresh - m_lo_thresh); //1530.0 / m_hi_thresh ;
	UPDATE_MULT;
		
	//depth map representation curve
	int i;
	for (i=0; i<10000; i++) {
	  	float v = i/2048.0;
	  	v = powf(v, 3)* 6;
	  	t_gamma[i] = v*6*256;
	}*/
	m_dataout = outlet_new(this->x_obj, 0);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_cvblob :: ~pix_cvblob()
{ }

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_cvblob :: processGrayImage(imageStruct &image)
{ 
  if ( image.xsize < 0 || image.ysize < 0 ) return;
  
  Mat imgMat2, input;
  std::vector<cv::Mat> split_array;
  std::vector<std::vector<cv::Point> > m_contours;
  std::vector<std::vector<int> > m_convexhulls;
  std::vector<int> m_area;
  
  //if ( image.csize == 1 ){
    imgMat2 = Mat( image.ysize, image.xsize, CV_8UC1, image.data, image.csize*image.xsize); // just transform imageStruct to cv::Mat without copying data
    input = imgMat2;
  /*} else if ( image.csize == 4 ){
    imgMat2 = Mat( image.ysize, image.xsize, CV_8UC4, image.data, image.csize*image.xsize); // just transform imageStruct to cv::Mat without copying data
    split(imgMat2,split_array);
    input = split_array[3]; // select alpha channel to find contours
  } else {
    error("suport only RGBA or GRAY image");
    return;
  }*/
  cv::Mat imgMat = input.clone(); // copy data because findContours will destroy it...

  m_contours.clear();
  m_convexhulls.clear();
  m_area.clear();
  
  /*****************/
  /* Find Contours */
  /*****************/

  std::vector<std::vector<cv::Point> > contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(imgMat, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////

void pix_cvblob :: obj_setupCallback(t_class *classPtr)
{
  /*CPPEXTERN_MSG1(classPtr, "epsilon",			epsilonMess,		double);
  CPPEXTERN_MSG1(classPtr, "area",				areaMess,			double);
  CPPEXTERN_MSG1(classPtr, "contours",			contoursMess,		double);
  CPPEXTERN_MSG1(classPtr, "cvblobOutput",		cvblobMess,			double);
  CPPEXTERN_MSG1(classPtr, "convexhulls",		convexhullsMess,	double);
  CPPEXTERN_MSG1(classPtr, "convexitydefects",	convexitydefectsMess, double);
  CPPEXTERN_MSG1(classPtr, "hierarchy_level",	hierarchyMess,		double);
  CPPEXTERN_MSG1(classPtr, "taboutput",			taboutputMess,		float);
  CPPEXTERN_MSG3(classPtr, "settab",			tableMess,			t_symbol*, t_symbol*, t_symbol*);*/
}


