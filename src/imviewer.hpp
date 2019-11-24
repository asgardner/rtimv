/** \file imviewer.hpp
  * \brief Declarations for the imviewer base class
  *
  * \author Jared R. Males (jaredmales@gmail.com)
  *
  */


#ifndef rtimv_imviewer_hpp
#define rtimv_imviewer_hpp

#include <cmath>
#include <iostream>
#include <vector>
#include <sys/time.h>
#include <sys/socket.h>

#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QWidget>
#include <QSocketNotifier>

#include "rtimvImage.hpp"

#include "pixaccess.h"

#include <cstdio>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/file.h>

#include <signal.h>

#include <ImageStreamIO.h>

#include "colorMaps.hpp"
#include "ImageStruct.hpp"

class imviewer : public QWidget
{
   Q_OBJECT

public:

   imviewer( const std::vector<std::string> & shkeys, 
             QWidget * Parent = 0, 
             Qt::WindowFlags f = 0
           );

 /** @name Image Data
     *    
     * @{
     */ 
protected:
   uint32_t m_nx {0}; ///< The number of pixels in the x (horizontal) direction
   uint32_t m_ny {0}; ///< The number of pixels in the y (vertical) direction

   std::vector<rtimvImage *> m_images;
      
public:   
   void setImsize(uint32_t x, uint32_t y); ///Changes the image size, but only if necessary.
   
   virtual void postSetImsize(); ///<to call after set_imsize to handle allocations for derived classes
      
   ///Get the number of x pixels
   /**
     * \returns the current vvalue of m_nx
     */ 
   uint32_t nx();

   ///Get the number of y pixels
   /**
     * \returns the current vvalue of m_ny
     */
   uint32_t ny();
   
   /// @}

protected:
   virtual void onConnect() {}
   
   /** @name Image Data Update Interval
     *    
     * @{
     */       
protected:
   QTimer m_timer; ///< When this times out imviewer checks for a new image.
   
   int m_timeout {100}; ///<The timeout for checking for a new images, ms.

protected slots:
   
   /// Function called by m_timer expiration.
   /** This is what actually checks for image data updates.
     */
   void timerout();
   
public:
   
   /// Set the image display timeout.
   /** This sets the display frame rate, e.g. a timeout of 100 msec will 
     * cause the display to update at 10 f.p.s.
     */ 
   void timeout(int to /**< [in] the new image display timeout*/);

   /// @}
   
   /** @name Calibrated Pixel Access
     * 
     * Settings to control which calibrations are applied, and provided acccess to calibrated pixels.
     * 
     * Calibrations include dark subtraction, reference subtraction, flat field, mask, and low and high pass filtering.
     * Note: only dark subtraction and masking are currently implemented.
     * 
     * The pixelF function pointer is used so that only a single `if-else` tree needs to be evaluated before 
     * iterating over all pixels.  The pixel() function returns a pointer to the static pixel_* function appropriate
     * for the current calibration settings.
     * 
     * @{
     */ 
public:
   /// The fuction pointer type for accessing pixels with calibrations applied.
   typedef float (*pixelF)(imviewer*, size_t);

   
//protected:   
   bool m_subtractDark {false};
   
   bool m_applyMask {false};
   
   bool m_applySatMask {false};

public:
   
   /// Returns a pointer to the static pixel value calculation function for the current calibration configuration
   /** Calibration configuration includes the value of m_subtractDark, m_applyMask.
     *
     * \returns a pointer to one of pixel_noCal, pixel_subDark, pixel_applyMask, pixel_subDarkApplyMask.
     */
   pixelF pixel();
      
   /// Access pixels with no calibrations applied.
   /** 
     * \returns the value of pixel idx
     */ 
   static float pixel_noCal( imviewer * imv, ///< [in] the imviewer instance to access
                             size_t idx      ///< [in] the linear pixel number to access
                           );
   
   /// Access pixels with dark subtraction applied.
   /** 
     * \returns the value of pixel idx after dark subtraction
     */
   static float pixel_subDark( imviewer * imv, ///< [in] the imviewer instance to access 
                               size_t idx      ///< [in] the linear pixel number to access
                             );
   
   /// Access pixels with the mask applied.
   /** 
     * \returns the value of pixel idx after applying the mask
     */
   static float pixel_applyMask( imviewer * imv, ///< [in] the imviewer instance to access
                                 size_t idx      ///< [in] the linear pixel number to access
                               );
   
   /// Access pixels with dark subtraction and masking applied.
   /** 
     * \returns the value of pixel idx after subtracting the dark and applying the mask
     */
   static float pixel_subDarkApplyMask( imviewer * imv, ///< [in] the imviewer instance to access 
                                        size_t idx      ///< [in] the linear pixel number to access
                                      );
   
   ///@}
   
   /** @name Colorbar Selection
     * 
     * @{
     */
public:
   typedef int (*pixelIndexF)(float);
   
   enum en_cbStretches{ stretchLinear,       ///< The pixel values are scaled linearly to between m_mindat and m_maxdat 
                        stretchLog,          ///< The pixel values are scaled logarithmically between m_mindat and m_maxdat 
                        stretchPow,          ///< the pixel values are scaled as \f$ 1000^p/1000 \f$ between m_mindat and m_maxdat
                        stretchSqrt,         ///< the pixel values are scaled as \f$ \sqrt(p) \f$ between m_mindat and m_maxdat
                        stretchSquare,       ///< the pixel values are scaled as \f$ p^2 \f$ between m_mindat and m_maxdat
                        cbStretches_max
                      };
   
protected:
   int m_mincol {0}; ///< The minimum index to use for the color table.
   
   int m_maxcol {253}; ///< The maximum index to use for the color table.
   
   int m_maskcol {254}; ///< The index in the color table to use for the mask color.
   
   int m_satcol {255}; ///< The index in the color table to use for the saturation color.

   int colorbar_mode {minmaxglobal};
   int m_cbStretch {stretchLinear};

   int current_colorbar {colorbarBone};

   QColor warning_color;
   
   

public:

   enum colorbars{colorbarGrey, colorbarJet, colorbarHot, colorbarBone, colorbarRed, colorbarGreen, colorbarBlue, colorbarMax};
   void load_colorbar(int);
   int get_current_colorbar(){return current_colorbar;}

   enum colorbar_modes{minmaxglobal, minmaxview, minmaxbox, user, colorbar_modes_max};
   void set_colorbar_mode(int mode){ colorbar_mode = mode;}
   int get_colorbar_mode(){return colorbar_mode;}

   
   void set_cbStretch(int);
   int get_cbStretch();

   pixelIndexF pixelIndex();
   
   static int pixelIndex_linear(float d);
   
   static int pixelIndex_log(float d);
   
   static int pixelIndex_pow(float d);
   
   static int pixelIndex_sqrt(float d);
   
   static int pixelIndex_square(float d);
   
   ///@}
   
   /** @name Colorbar Scale Control
     * 
     * @{
     */

protected:
   /*** Color Map ***/
   float m_mindat;  ///< The minimum data value used for scaling
      
   float m_maxdat; ///< The maximum data valuse used for scaling

   bool m_autoScale {false};

public:

   
   void mindat(float md);
      
   float mindat();
      
   void maxdat(float md);
      
   float maxdat();
   
   void bias(float b);

   float bias();
   
   void bias_rel(float b);

   float bias_rel();
   
   void contrast(float c);

   float contrast();
   
   void contrast_rel(float cr);
   
   float contrast_rel();

protected:
   
   /// Calculates the color map index of a value.
   /** Returned value includes the effect of the current color bar stretch (m_mindat and m_maxdat) 
     * and the current color bar scale (m_colorbarScale) 
     */
   int calcPixIndex(float d);

   ///@}
   
   /** @name Image Filtering
     * 
     * @{
     */

      float * m_lowPassFiltered {nullptr};
      
      bool m_applyLPFilter;
      
      int m_lpFilterType;
   
   ///@} -- filtering
   
   //****** The display *************
protected:
      
      
   QImage * m_qim {nullptr}; ///<A QT image, used to store the color-map encoded data
      
   QPixmap m_qpm; ///<A QT pixmap, used to prep the QImage for display.

   bool amChangingimdata;
   
public:
      
   ///Get the QPixMap pointer
   QPixmap * getPixmap(){return &m_qpm;}

   
   ///Updates the QImage and basic statistics after a new image.
   /** \param newdata determines whether statistics are calculated (true) or not (false).
     */
   void changeImdata(bool newdata = false);

   virtual void postChangeImdata(); ///<to call after change imdata does its work.

   

   protected:
      float sat_level {1e30};
      int saturated {0};


   /* Image Stats */
   protected:
      float imdat_min;
      float imdat_max;

   public:
      float get_imdat_min(){return imdat_min;}
      float get_imdat_max(){return imdat_max;}

   /*** Abstract Zoom ***/
   protected:
      float m_zoomLevel {1};
      float m_zoomLevelMin {0.125};
      float m_zoomLevelMax {64};

   public:
      float zoomLevel(){return m_zoomLevel;}
      float zoomLevelMin(){return m_zoomLevelMin;}
      float zoomLevelMax(){return m_zoomLevelMax;}

      //void set_ZoomLevel(int zlint);
      void zoomLevel(float zl);
      virtual void post_zoomLevel();

  
   /** @name A User Defined Region
     *
     * @{
     */
protected:
   int userBoxActive {0};

   //ImageStreamIO images are sized in uint32_t, so we need these big enough for signed comparisons without wraparound
   int64_t userBox_i0;
   int64_t userBox_i1;
   int64_t userBox_j0;
   int64_t userBox_j1;

   int64_t guideBox_i0;
   int64_t guideBox_i1;
   int64_t guideBox_j0;
   int64_t guideBox_j1;

   float userBox_max;
   float userBox_min;

public:
   int getUserBoxActive(){ return userBoxActive; }
   
   void setUserBoxActive(bool usba);
   
   ///@}


   /*** Real Time Controls ***/   
protected:
      bool RealTimeEnabled {true}; ///<Controls whether imviewer is using real-time data.
      bool RealTimeStopped {false}; ///<Set when user temporarily freezes real-time data viewing.

public:
      void set_RealTimeEnabled(int);
      void set_RealTimeStopped(int);

      virtual void updateFPS();///<Called whenever the displayed image updates its FPS.
      virtual void updateAge();///<Called whenever the displayed image updates its Age.

public:
   static void st_handleSigSegv( int signum,
                                 siginfo_t *siginf,
                                 void *ucont
                               );

public slots:
   void handleSigSegv();

private:   
   static int sigsegvFd[2];

   QSocketNotifier *snSegv;
   
   
   
   
   
};

#endif //rtimv_imviewer_hpp
