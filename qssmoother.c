/*!
 * @file qssmother.c   
 * @author J. Camilo Gomez C.
 * @note This file is part of the qTools distribution.
 **/

#include "qssmoother.h"
#include "qltisys.h"
    
struct qSmoother_Vtbl_s {
    float (*perform)( _qSSmoother_t *f, float x );
    int (*setup)( _qSSmoother_t *f, float *param, float *window, size_t wsize );
};

static float qSSmoother_Abs( float x );
static void qSSmoother_WindowSet( float *w, size_t wsize, float x );
static int qSSmoother_Setup_LPF1( _qSSmoother_t *f, float *param, float *window, size_t wsize );
static int qSSmoother_Setup_LPF2( _qSSmoother_t *f, float *param, float *window, size_t wsize );
static int qSSmoother_Setup_MWM( _qSSmoother_t *f, float *param, float *window, size_t wsize );
static int qSSmoother_Setup_MWM2( _qSSmoother_t *f, float *param, float *window, size_t wsize );
static int qSSmoother_Setup_MWOR( _qSSmoother_t *f, float *param, float *window, size_t wsize );
static int qSSmoother_Setup_MWOR2( _qSSmoother_t *f, float *param, float *window, size_t wsize );
static int qSSmoother_Setup_GAUSSIAN( _qSSmoother_t *f, float *param, float *window, size_t wsize );
static int qSSmoother_Setup_KALMAN( _qSSmoother_t *f, float *param, float *window, size_t wsize );
static float qSSmoother_Filter_LPF1( _qSSmoother_t *f, float x );
static float qSSmoother_Filter_LPF2( _qSSmoother_t *f, float x );
static float qSSmoother_Filter_MWM( _qSSmoother_t *f, float x );
static float qSSmoother_Filter_MWM2( _qSSmoother_t *f, float x );
static float qSSmoother_Filter_MWOR( _qSSmoother_t *f, float x );
static float qSSmoother_Filter_MWOR2( _qSSmoother_t *f, float x );
static float qSSmoother_Filter_GAUSSIAN( _qSSmoother_t *f, float x ) ;
static float qSSmoother_Filter_KALMAN( _qSSmoother_t *f, float x );

/*============================================================================*/
int qSSmoother_Setup( qSSmootherPtr_t s, qSSmoother_Type_t type, float *param, float *window, size_t wsize )
{
    static struct qSmoother_Vtbl_s qSmoother_Vtbl[ 8 ] = {
        { &qSSmoother_Filter_LPF1       ,&qSSmoother_Setup_LPF1 }, 
        { &qSSmoother_Filter_LPF2       ,&qSSmoother_Setup_LPF2 },
        { &qSSmoother_Filter_MWM        ,&qSSmoother_Setup_MWM },
        { &qSSmoother_Filter_MWM2       ,&qSSmoother_Setup_MWM2 },
        { &qSSmoother_Filter_MWOR       ,&qSSmoother_Setup_MWOR },
        { &qSSmoother_Filter_MWOR2      ,&qSSmoother_Setup_MWOR2 },
        { &qSSmoother_Filter_GAUSSIAN   ,&qSSmoother_Setup_GAUSSIAN},
        { &qSSmoother_Filter_KALMAN     ,&qSSmoother_Setup_KALMAN},
    };    
    int retVal = 0;
    
    if ( ( s != NULL ) && ( (size_t)type < ( sizeof(qSmoother_Vtbl)/sizeof(qSmoother_Vtbl[ 0 ] ) ) ) ){
        _qSSmoother_t *self = (_qSSmoother_t*)s;
        self->vt = &qSmoother_Vtbl[ type ];
        retVal = qSmoother_Vtbl[ type ].setup( s, param, window, wsize );
    }
    return retVal;
}
/*============================================================================*/
static int qSSmoother_Setup_LPF1( _qSSmoother_t *f, float *param, float *window, size_t wsize )
{
    int retVal = 0;
    float alpha = param[ 0 ];
    
    if ( ( alpha > 0.0f ) && ( alpha < 1.0f ) ) {
        /*cstat -MISRAC2012-Rule-11.3 -CERT-EXP39-C_d -CERT-EXP36-C_a*/
        qSSmoother_LPF1_t *s = (qSSmoother_LPF1_t*)f;
        /*cstat +MISRAC2012-Rule-11.3 +CERT-EXP39-C_d +CERT-EXP36-C_a*/
        s->alpha = alpha;
        retVal = qSSmoother_Reset( s );
        (void)window;
        (void)wsize;
    }
    return retVal;
}
/*============================================================================*/
static int qSSmoother_Setup_LPF2( _qSSmoother_t *f, float *param, float *window, size_t wsize )
{
    int retVal = 0;
    float alpha = param[ 0 ];

    if ( ( alpha > 0.0f ) && ( alpha < 1.0f ) ) {
        /*cstat -MISRAC2012-Rule-11.3 -CERT-EXP39-C_d -CERT-EXP36-C_a*/
        qSSmoother_LPF2_t *s = (qSSmoother_LPF2_t*)f;
        /*cstat +MISRAC2012-Rule-11.3 +CERT-EXP39-C_d +CERT-EXP36-C_a*/
        float aa, p1, r;
        aa = alpha*alpha;
        /*cstat -MISRAC2012-Dir-4.11_b*/
        p1 = sqrtf( 2.0f*alpha ); /*arg always positive*/
        /*cstat +MISRAC2012-Dir-4.11_b*/
        r = 1.0f + p1 + aa;
        s->k = aa/r;
        s->a1 = 2.0f*( aa - 1.0f )/r;
        s->a2 = ( 1.0f - p1 + aa )/r;
        s->b1 = 2.0f*s->k;        
        retVal = qSSmoother_Reset( s );
        (void)window;
        (void)wsize;
    }
    return retVal;
}
/*============================================================================*/
static int qSSmoother_Setup_MWM( _qSSmoother_t *f, float *param, float *window, size_t wsize )
{
    int retVal = 0;
    if ( ( NULL != window ) && ( wsize > 0u ) ) {
        /*cstat -MISRAC2012-Rule-11.3 -CERT-EXP39-C_d -CERT-EXP36-C_a*/
        qSSmoother_MWM_t *s = (qSSmoother_MWM_t*)f;
        /*cstat +MISRAC2012-Rule-11.3 +CERT-EXP39-C_d +CERT-EXP36-C_a*/
        s->w = window;
        s->wsize = wsize;
        retVal = qSSmoother_Reset( s );
        (void)param;
    }
    return retVal;
}
/*============================================================================*/
static int qSSmoother_Setup_MWM2( _qSSmoother_t *f, float *param, float *window, size_t wsize )
{
    int retVal = 0;
    if ( ( NULL != window ) && ( wsize > 0u ) ) {
        /*cstat -MISRAC2012-Rule-11.3 -CERT-EXP39-C_d -CERT-EXP36-C_a*/
        qSSmoother_MWM2_t *s = (qSSmoother_MWM2_t*)f;
        /*cstat +MISRAC2012-Rule-11.3 +CERT-EXP39-C_d +CERT-EXP36-C_a*/
        qTDL_Setup( &s->tdl, window, wsize, 0.0f );
        s->sum  = 0.0f;
        retVal = qSSmoother_Reset( s );
        (void)param;
    }
    return retVal;
}
/*============================================================================*/
static int qSSmoother_Setup_MWOR( _qSSmoother_t *f, float *param, float *window, size_t wsize )
{
    int retVal = 0;
    float alpha = param[ 0 ];
    if ( ( NULL != window ) && ( wsize > 0u ) && ( alpha > 0.0f ) && ( alpha < 1.0f ) ) {
        /*cstat -MISRAC2012-Rule-11.3 -CERT-EXP39-C_d -CERT-EXP36-C_a*/
        qSSmoother_MWOR_t *s = (qSSmoother_MWOR_t*)f;
        /*cstat +MISRAC2012-Rule-11.3 +CERT-EXP39-C_d +CERT-EXP36-C_a*/
        s->w = window;
        s->wsize = wsize;
        s->alpha  = alpha;
        retVal = qSSmoother_Reset( s ); ;
    }
    return retVal;
}
/*============================================================================*/
static int qSSmoother_Setup_MWOR2( _qSSmoother_t *f, float *param, float *window, size_t wsize )
{
    int retVal = 0;
    float alpha = param[ 0 ];
    if ( ( NULL != window ) && ( wsize > 0u ) && ( alpha > 0.0f ) && ( alpha < 1.0f ) ) {
        /*cstat -MISRAC2012-Rule-11.3 -CERT-EXP39-C_d -CERT-EXP36-C_a*/
        qSSmoother_MWOR2_t *s = (qSSmoother_MWOR2_t*)f;
        /*cstat +MISRAC2012-Rule-11.3 +CERT-EXP39-C_d +CERT-EXP36-C_a*/
        s->alpha  = alpha;
        qTDL_Setup( &s->tdl, window, wsize, 0.0f );
        s->sum = 0.0f;
        s->m = 0.0f;
        retVal = qSSmoother_Reset( s ); ;
    }
    return retVal;
}
/*============================================================================*/
static int qSSmoother_Setup_GAUSSIAN( _qSSmoother_t *f, float *param, float *window, size_t wsize )
{
    int retVal = 0;
    float sigma = param[ 0 ];
    /*cstat -CERT-FLP34-C*/
    size_t c = (size_t)param[ 1 ];
    /*cstat +CERT-FLP34-C*/
    size_t ws = wsize/2u;

    if ( ( NULL != window ) && ( wsize > 0u ) && ( c < ws ) && ( sigma > 0.0f ) ) {
        /*cstat -MISRAC2012-Rule-11.3 -CERT-EXP39-C_d -CERT-EXP36-C_a*/
        qSSmoother_GAUSSIAN_t *s = (qSSmoother_GAUSSIAN_t*)f;
        /*cstat +MISRAC2012-Rule-11.3 +CERT-EXP39-C_d +CERT-EXP36-C_a*/
        float *kernel = &window[ ws ];
        float r, sum = 0.0f;
        size_t i;
        float L, center; 
        /*cstat -CERT-FLP36-C -MISRAC2012-Rule-10.8*/
        L = (float)(wsize - 1u)/2.0f;
        center = (float)c - L;
        r =  2.0f*sigma*sigma ;
        for ( i = 0u ; i < ws ; ++i ) {
            float d = (float)i - L;  /*symmetry*/     
            d -= center;     
            kernel[ i ] =  expf( -(d*d)/r );            
            sum += kernel[ i ];
        }        
        /*cstat +CERT-FLP36-C +MISRAC2012-Rule-10.8*/
        for ( i = 0u ; i < ws ; ++i ) {
            kernel[ i ] /= sum;
        }
        
        s->w = window;
        s->k = kernel;
        s->wsize = ws;
        retVal = qSSmoother_Reset( s );
    }
    return retVal;
}
/*============================================================================*/
static int qSSmoother_Setup_KALMAN( _qSSmoother_t *f, float *param, float *window, size_t wsize )
{
    int retVal = 0;
    float p = param[ 0 ];
    float q = param[ 1 ];
    float r = param[ 2 ];
    if ( ( p > 0.0f ) && ( q > 0.0f ) && ( r > 0.0f ) ) {
        /*cstat -MISRAC2012-Rule-11.3 -CERT-EXP39-C_d -CERT-EXP36-C_a*/
        qSSmoother_KALMAN_t *s = (qSSmoother_KALMAN_t*)f;
        /*cstat +MISRAC2012-Rule-11.3 +CERT-EXP39-C_d +CERT-EXP36-C_a*/
        s->p = p; 
        s->q = q;
        s->r = r;
        s->A = 1.0f;
        s->H = 1.0f;
        retVal = qSSmoother_Reset( s );
        (void)window;
        (void)wsize;
    }
    return retVal;
}
/*============================================================================*/
static float qSSmoother_Abs( float x )
{
    return ( x < 0.0f )? -x : x; 
}
/*============================================================================*/
static void qSSmoother_WindowSet( float *w, size_t wsize, float x )
{
    size_t i;
    for ( i = 0 ; i < wsize ; ++i ) {
        w[ i ] = x;
    }    
}
/*============================================================================*/
int qSSmoother_IsInitialized( qSSmootherPtr_t s )
{
    int retVal = 0;
    if ( NULL != s ) {
        _qSSmoother_t *f = (_qSSmoother_t*)s;
        retVal = (int)( NULL != f->vt );
    }    
    return retVal;
} 
/*============================================================================*/
int qSSmoother_Reset( qSSmootherPtr_t s ) 
{
    int retVal = 0;
    if ( NULL != s ) {
        _qSSmoother_t *f = (_qSSmoother_t*)s;
        f->init = 1u;
        retVal = 1;
    }
    return retVal;
}
/*============================================================================*/
float qSSmoother_Perform( qSSmootherPtr_t s, float x )
{
    float out = x;
    if ( NULL != s ) {
        _qSSmoother_t *f = (_qSSmoother_t*)s;
        /*cstat -MISRAC2012-Rule-11.5 -CERT-EXP36-C_b*/
        struct qSmoother_Vtbl_s *vt = f->vt;
        /*cstat +MISRAC2012-Rule-11.5 +CERT-EXP36-C_b*/
        if ( NULL != vt->perform ) {
            out = vt->perform( f, x );
        }
    }
    return out;
}
/*============================================================================*/
static float qSSmoother_Filter_LPF1( _qSSmoother_t *f, float x ) 
{
    float y;
    /*cstat -CERT-EXP36-C_a -MISRAC2012-Rule-11.3 -CERT-EXP39-C_d*/
    qSSmoother_LPF1_t *s = (qSSmoother_LPF1_t*)f;
    /*cstat +CERT-EXP36-C_a +MISRAC2012-Rule-11.3 +CERT-EXP39-C_d*/
    if ( 1u == f->init ) {
        s->y1 = x;
        f->init = 0u;
    }
    y = x + ( s->alpha*( s->y1 - x ) ) ;
    s->y1 = y;    
    
    return y;
}
/*============================================================================*/
static float qSSmoother_Filter_LPF2( _qSSmoother_t *f, float x ) 
{
    float y;
    /*cstat -CERT-EXP36-C_a -MISRAC2012-Rule-11.3 -CERT-EXP39-C_d*/
    qSSmoother_LPF2_t *s = (qSSmoother_LPF2_t*)f;
    /*cstat +CERT-EXP36-C_a +MISRAC2012-Rule-11.3 +CERT-EXP39-C_d*/
    if ( 1u == f->init ) {
        s->y1 = x;
        s->y2 = x;
        s->x1 = x;
        s->x2 = x;
        f->init = 0u;
    }     
    y = ( s->k*x  ) + ( s->b1*s->x1 ) + ( s->k*s->x2 ) - ( s->a1*s->y1 ) - ( s->a2*s->y2 );
    s->x2 = s->x1;
    s->x1 = x;
    s->y2 = s->y1;
    s->y1 = y;    
    
    return y;
}
/*============================================================================*/
static float qSSmoother_Filter_MWM( _qSSmoother_t *f, float x ) 
{
    /*cstat -CERT-EXP36-C_a -MISRAC2012-Rule-11.3 -CERT-EXP39-C_d*/
    qSSmoother_MWM_t *s = (qSSmoother_MWM_t*)f;
    /*cstat +CERT-EXP36-C_a +MISRAC2012-Rule-11.3 +CERT-EXP39-C_d*/
    if ( 1u == f->init ) {
        qSSmoother_WindowSet( s->w, s->wsize, x );
        f->init = 0u;
    }        
    /*cstat -CERT-FLP36-C*/
    return qLTISys_DiscreteFIRUpdate( s->w, NULL, s->wsize, x ) / (float)s->wsize; 
    /*cstat +CERT-FLP36-C*/
}
/*============================================================================*/
static float qSSmoother_Filter_MWM2( _qSSmoother_t *f, float x ) 
{
    /*cstat -CERT-EXP36-C_a -MISRAC2012-Rule-11.3 -CERT-EXP39-C_d -CERT-FLP36-C*/
    qSSmoother_MWM2_t *s = (qSSmoother_MWM2_t*)f;
    float wsize = (float)s->tdl.itemcount;
    /*cstat +CERT-EXP36-C_a +MISRAC2012-Rule-11.3 +CERT-EXP39-C_d +CERT-FLP36-C*/
    if ( 1u == f->init ) {
        qTDL_Flush( &s->tdl, x );
        s->sum = x*wsize;
        f->init = 0u;
    }     
    s->sum += x - qTDL_GetOldest( &s->tdl );
    qTDL_InsertSample( &s->tdl, x );
    return s->sum/wsize;
}
/*============================================================================*/
static float qSSmoother_Filter_MWOR( _qSSmoother_t *f, float x ) 
{
    float m;
    /*cstat -CERT-EXP36-C_a -MISRAC2012-Rule-11.3 -CERT-EXP39-C_d*/
    qSSmoother_MWOR_t *s = (qSSmoother_MWOR_t*)f;
    /*cstat +CERT-EXP36-C_a +MISRAC2012-Rule-11.3 +CERT-EXP39-C_d*/
    if ( 1u == f->init ) {
        qSSmoother_WindowSet( s->w, s->wsize, x );
        s->m = x; 
        f->init = 0u;
    }        

    m = qLTISys_DiscreteFIRUpdate( s->w, NULL, s->wsize, x ) - x; /*shift, sum and compensate*/
    if ( qSSmoother_Abs( s->m - x )  > ( s->alpha*qSSmoother_Abs( s->m ) ) ) { /*is it an outlier?*/
        s->w[ 0 ] = s->m; /*replace the outlier with the dynamic median*/
    }
    /*cstat -CERT-FLP36-C*/
    s->m = ( m + s->w[ 0 ] ) / (float)s->wsize;  /*compute new mean for next iteration*/   
    /*cstat +CERT-FLP36-C*/
    return s->w[ 0 ];
}
/*============================================================================*/
static float qSSmoother_Filter_MWOR2( _qSSmoother_t *f, float x )
{
    /*cstat -CERT-EXP36-C_a -MISRAC2012-Rule-11.3 -CERT-EXP39-C_d -CERT-FLP36-C*/
    qSSmoother_MWOR2_t *s = (qSSmoother_MWOR2_t*)f;  
    float wsize = (float)s->tdl.itemcount;
    /*cstat +CERT-EXP36-C_a +MISRAC2012-Rule-11.3 +CERT-EXP39-C_d +CERT-FLP36-C*/
    if( 1u == f->init ) {
        qTDL_Flush( &s->tdl, x );
        s->sum = wsize*x;
        s->m = x;
        f->init = 0u;   
    }
    if ( qSSmoother_Abs( s->m - x )  > ( s->alpha*qSSmoother_Abs( s->m ) ) ) { /*is it an outlier?*/
        x = s->m; /*replace the outlier with the dynamic median*/
    }
    s->sum += x - qTDL_GetOldest( &s->tdl );
    s->m = s->sum/wsize;
    qTDL_InsertSample( &s->tdl, x );
    return x;
}
/*============================================================================*/
static float qSSmoother_Filter_GAUSSIAN( _qSSmoother_t *f, float x ) 
{
    /*cstat -CERT-EXP36-C_a -MISRAC2012-Rule-11.3 -CERT-EXP39-C_d*/
    qSSmoother_GAUSSIAN_t *s = (qSSmoother_GAUSSIAN_t*)f;
    /*cstat +CERT-EXP36-C_a +MISRAC2012-Rule-11.3 +CERT-EXP39-C_d*/
    
    if ( 1u == f->init ) {
        qSSmoother_WindowSet( s->w, s->wsize, x );
        f->init = 0u;
    }   
    return qLTISys_DiscreteFIRUpdate( s->w, s->k, s->wsize, x );
}
/*============================================================================*/
static float qSSmoother_Filter_KALMAN( _qSSmoother_t *f, float x )
{
    /*cstat -CERT-EXP36-C_a -MISRAC2012-Rule-11.3 -CERT-EXP39-C_d*/
    qSSmoother_KALMAN_t *s = (qSSmoother_KALMAN_t*)f;
    /*cstat +CERT-EXP36-C_a +MISRAC2012-Rule-11.3 +CERT-EXP39-C_d*/    
    float pH;
    if ( 1u == f->init ) {
        s->x = x;
        f->init = 0u;
    }     
    /* Predict */
    s->x = s->A*s->x;
    s->p = ( s->A*s->A*s->p ) + s->q;  /* p(n|n-1)=A^2*p(n-1|n-1)+q */
    /* Measurement */
    pH = s->p*s->H;
    s->gain =  pH/( s->r + ( s->H*pH ) );
    s->x += s->gain*( x - ( s->H*s->x ) );
    s->p = ( 1.0f - ( s->gain*s->H ) )*s->p;
    return s->x;      
}
/*============================================================================*/