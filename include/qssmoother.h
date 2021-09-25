/*!
 * @file qssmoother.h
 * @author J. Camilo Gomez C.
 * @version 1.02
 * @note This file is part of the qTools distribution.
 * @brief API to smooth noisy signals.
 **/

#ifndef QSIGNAL_SMOOTHER
#define QSIGNAL_SMOOTHER

#ifdef __cplusplus
extern "C" {
#endif

    #include <stdlib.h>
    #include <stdint.h>
    #include <float.h>
    #include <math.h>
    #include "qtdl.h"

    typedef enum{
        QSSMOOTHER_TYPE_LPF1 = 0,
        QSSMOOTHER_TYPE_LPF2,
        QSSMOOTHER_TYPE_MWM,
        QSSMOOTHER_TYPE_MWM2,
        QSSMOOTHER_TYPE_MWOR,
        QSSMOOTHER_TYPE_MWOR2,
        QSSMOOTHER_TYPE_GAUSSIAN,
        QSSMOOTHER_TYPE_KALMAN,
        QSSMOOTHER_TYPE_EXPW,
    }qSSmoother_Type_t;

    typedef void* qSSmootherPtr_t;

    
    /*! @cond  */
    /*abstract class*/
    typedef struct _qSSmoother_s 
    {   
        void *vt;
        uint8_t init;
    } _qSSmoother_t;
    /*! @endcond  */

    typedef struct 
    {   
        /*! @cond  */
        _qSSmoother_t f;
        float alpha, y1;
        /*! @endcond  */
    } qSSmoother_LPF1_t;

    typedef struct 
    {
        /*! @cond  */
        _qSSmoother_t f;
        float y1, y2, x1, x2;
        float k, a1, a2, b1;
        /*! @endcond  */
    } qSSmoother_LPF2_t;

    typedef struct
    {
        /*! @cond  */
        _qSSmoother_t f;
        float *w;
        size_t wsize;
        /*! @endcond  */
    } qSSmoother_MWM_t;

    typedef struct
    {
        /*! @cond  */
        _qSSmoother_t f;
        qTDL_t tdl;
        float sum;
        /*! @endcond  */
    } qSSmoother_MWM2_t;

    typedef struct
    {
        /*! @cond  */
        _qSSmoother_t f;
        float *w, m, alpha;
        size_t wsize;
        /*! @endcond  */
    } qSSmoother_MWOR_t;    
 
    typedef struct
    {
        /*! @cond  */
        _qSSmoother_t f;
        qTDL_t tdl;
        float sum, m, alpha;
        /*! @endcond  */
    } qSSmoother_MWOR2_t;  

    typedef struct
    {
        /*! @cond  */
        _qSSmoother_t f;
        float *w, *k;
        size_t wsize;
        /*! @endcond  */
    } qSSmoother_GAUSSIAN_t;    

    typedef struct
    {
        /*! @cond  */
        _qSSmoother_t f;
        float lambda, m, w;
        /*! @endcond  */
    } qSSmoother_EXPW_t; 

    typedef struct
    {
        /*! @cond  */
        _qSSmoother_t f;
        float x;  /* state */
        float A;  /* x(n)=A*x(n-1)+u(n),u(n)~N(0,q) */
        float H;  /* z(n)=H*x(n)+w(n),w(n)~N(0,r)   */
        float q;  /* process(predict) noise convariance */
        float r;  /* measure noise convariance */
        float p;  /* estimated error convariance */
        float gain;
        /*! @endcond  */
    } qSSmoother_KALMAN_t; 

    /**
    * @brief Check if the smoother filter is initialized.
    * @param[in] s A pointer to the signal smoother instance.
    * @return 1 if the smoother has been initialized, otherwise return 0.
    */        
    int qSSmoother_IsInitialized( qSSmootherPtr_t s );

    /**
    * @brief Reset the the smoother filter.
    * @param[in] s A pointer to the signal smoother instance.
    * @return 1 on success, otherwise return 0.
    */     
    int qSSmoother_Reset( qSSmootherPtr_t s );

    /**
    * @brief Perform the smooth operation recursively for the input signal @a x.
    * @param[in] s A pointer to the signal smoother instance.
    * @param[in] x A sample of the input signal.
    * @return The smoothed output.
    */    
    float qSSmoother_Perform( qSSmootherPtr_t s, float x );

    /**
    * @brief Setup an initialize smoother filter.
    * @param[in] s A pointer to the signal smoother instance.
    * @param[in] type The filter type. Use one of the following values:
    * 
    * - ::QSSMOOTHER_TYPE_LPF1.
    * 
    * - ::QSSMOOTHER_TYPE_LPF2.
    * 
    * - ::QSSMOOTHER_TYPE_MWM.
    * 
    * - ::QSSMOOTHER_TYPE_MWM2.
    * 
    * - ::QSSMOOTHER_TYPE_MWOR.
    * 
    * - ::QSSMOOTHER_TYPE_MWOR2.
    * 
    * - ::QSSMOOTHER_TYPE_GAUSSIAN.
    * 
    * - ::QSSMOOTHER_TYPE_KALMAN.
    * 
    * - ::QSSMOOTHER_TYPE_EXPW.
    * 
    * @param[in] param The smoother parameters. Depends of the type selected:
    * 
    * if ::QSSMOOTHER_TYPE_LPF1, a value between  [ 0 < alpha < 1 ] 
    * 
    * if ::QSSMOOTHER_TYPE_LPF2, a value between  [ 0 < alpha < 1 ]
    * 
    * if ::QSSMOOTHER_TYPE_MWM, can be ignored. Pass NULL as argument.
    * 
    * if ::QSSMOOTHER_TYPE_MWM2, can be ignored. Pass NULL as argument.
    * 
    * if ::QSSMOOTHER_TYPE_MWOR, a value between  [ 0 < alpha < 1 ]
    * 
    * if ::QSSMOOTHER_TYPE_MWOR2, a value between  [ 0 < alpha < 1 ]
    * 
    * if ::QSSMOOTHER_TYPE_GAUSSIAN, an array with two values. The first element
    * with the Standard deviation [ sigma > 0 ]. The second element withthe 
    * offset of the gaussian center. [ 0 < pos < (wsize-1) ].
    * 
    * if ::QSSMOOTHER_TYPE_KALMAN, an array with three values. The first element
    * with the initial estimated error convariance. The second element with the 
    * process(predict) noise convariance. The third element with the measure 
    * noise convariance
    * 
    * if ::QSSMOOTHER_TYPE_EXPW, a value between  [ 0 < lambda < 1 ] that represents
    * the forgetting factor. 
    * 
    * @param[in] window The filter window and coefficients. Depends of the type 
    * selected:
    * 
    * if ::QSSMOOTHER_TYPE_LPF1, can be ignored. Pass NULL as argument.
    * 
    * if ::QSSMOOTHER_TYPE_LPF2, can be ignored. Pass NULL as argument.
    * 
    * if ::QSSMOOTHER_TYPE_MWM, An array of @a wsize elements.
    * 
    * if ::QSSMOOTHER_TYPE_MWM2, An array of @a wsize elements.
    * 
    * if ::QSSMOOTHER_TYPE_MWOR, An array of @a wsize elements.
    * 
    * if ::QSSMOOTHER_TYPE_MWOR2, An array of @a wsize elements.
    * 
    * if ::QSSMOOTHER_TYPE_GAUSSIAN, An array of @a wsize to hold both, the 
    * window and the gaussian kernel coefficients. 
    * 
    * if ::QSSMOOTHER_TYPE_KALMAN, can be ignored. Pass NULL as argument.
    * 
    * if ::QSSMOOTHER_TYPE_EXPW, can be ignored. Pass NULL as argument.
    * 
    * @param[in] wsize If used, the number of elements in @a window, otherwise
    * pass 0uL as argument.
    * @return 1 on success, otherwise return 0.
    */  
    int qSSmoother_Setup( qSSmootherPtr_t s, qSSmoother_Type_t type, float *param, float *window, size_t wsize );


#ifdef __cplusplus
}
#endif

#endif