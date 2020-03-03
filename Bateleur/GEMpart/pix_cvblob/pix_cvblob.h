/*-----------------------------------------------------------------
LOG
GEM - Graphics Environment for Multimedia

Clamp pixel values to a threshold

Copyright (c) 1997-1998 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
Copyright (c) 2002 James Tittle & Chris Clepper
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_pix_cvblob_H_
#define _INCLUDE__GEM_PIXES_pix_cvblob_H_

#include "Base/GemPixObj.h"
#include "cv.hpp"
#include "opencv2/legacy/legacy.hpp"

#ifdef _WIN32
#include "stdint.h"
#endif

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_cvblob
    
    Clamp pixel values to a threshold

KEYWORDS
    pix
    
DESCRIPTION

    Inlet for a list - "vec_thresh"
    Inlet for a float - "ft1"
    
    "vec_thresh" - The threshold vector
    "ft1" - Set all thresholds to one value
   
-----------------------------------------------------------------*/
#ifdef _WIN32
class GEM_EXPORT pix_cvblob : public GemPixObj
#else
class GEM_EXTERN pix_cvblob : public GemPixObj
#endif
{
    CPPEXTERN_HEADER(pix_cvblob, GemPixObj);

    public:

        //////////
        // Constructor
			pix_cvblob(/*t_floatarg hi_thresh, t_floatarg lo_thresh*/);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_cvblob();

    	//////////
    	// Do the processing
		virtual void 	processGrayImage(imageStruct &image);
    	//virtual void 	processRGBAImage(imageStruct &image);
    	//virtual void 	processYUVImage(imageStruct &image); // not done yet!
    	/*virtual void 	processRGBAImage(imageStruct &image) {};
    	virtual void 	processGrayImage(imageStruct &image) {};
		virtual void 	processYUVImage(imageStruct &image) {};*/

    	//////////

		//////////
		// Set the new threshold value
		/*void	    	floatHiThreshMess(float hi_thresh);
		void	    	floatLoThreshMess(float lo_thresh);
		void	    	floatModeMess(float arg);
		void	    	floatActiveMess(float arg);
		void	    	floatGrayMess(float arg);*/

		// distance thresholds - min < passing < max
		/*float m_hi_thresh;
		float m_lo_thresh;
		
		float t_mult;
		
		// if true convert into "human colorspace"
		bool m_active;

		// if true depth input is in mm, false if kinect raw 11bit data
		bool m_mode;
		
		// return gray image instead of rgba
		bool m_gray;*/
		
		// convertion curves
		//uint16_t t_gamma[10000];

	private:
    
    	/*//////////
    	// Static member functions
    	static void 	floatHiThreshMessCallback(void *data, t_floatarg arg);
    	static void 	floatLoThreshMessCallback(void *data, t_floatarg arg);
    	static void 	floatModeMessCallback(void *data, t_floatarg arg);
    	static void 	floatActiveMessCallback(void *data, t_floatarg arg);
    	static void 	floatGrayMessCallback(void *data, t_floatarg arg);*/
    	
		t_outlet *m_dataout; // blobs outlet
};

#endif	// for header file
