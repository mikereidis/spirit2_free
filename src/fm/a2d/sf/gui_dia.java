
package fm.a2d.sf;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.MotionEvent;
import android.widget.ImageView;
import android.widget.ImageView.ScaleType;
import android.widget.RelativeLayout;
import android.graphics.Canvas;

public class gui_dia extends RelativeLayout implements OnGestureListener {

  private static int    m_obinits = 1;

  private GestureDetector   gest_det;
  private int               m_width = 0;
  private int               m_height= 0;

  private ImageView         iv_dial;
  private Bitmap            bmp_dial;

  private double corner_lo = 0.8;//0.85;
  private double corner_hi = 0.2;//0.15;

  private boolean is_down_powr = false;
  private boolean is_down_dial = false;
  private boolean is_down_next = false;
  private boolean is_down_prev = false;

    // Callback interface:
  private gui_dia_listener m_listener;
  interface gui_dia_listener {
    public boolean  state_chngd ();                                         // Previously had (boolean newstate), but now just an externally handled toggle
    public boolean  dial_chngd (double angle);
    public boolean  freq_go ();
    public boolean  prev_go ();
    public boolean  next_go ();
  }


    // Dial Constructor
  public gui_dia (Context context, int dial_id, int needle_id, int width, int height) {
    super (context);

    com_uti.logd ("m_obinits: " + m_obinits++);

    com_uti.logd ("dial_id: " + dial_id + "  needle_id: " + needle_id + "  width: " + width + "  height: " + height);

    m_width = width;                                                    // Save width and height
    m_height = height;

                                                                        // Load dial image
    Bitmap src_bmp_dial1 = BitmapFactory.decodeResource (context.getResources (), dial_id);
    Bitmap src_bmp_dial2 = null;
    Bitmap src_bmp_dial;
    if (needle_id >= 0)
      src_bmp_dial2 = BitmapFactory.decodeResource (context.getResources (), needle_id);
    if (needle_id >= 0)
      src_bmp_dial = bitmaps_combine (src_bmp_dial1, src_bmp_dial2);
    else
      src_bmp_dial = src_bmp_dial1;

    double scale_width = ((double) width) / src_bmp_dial.getWidth ();
    double scale_height = ((double) height) / src_bmp_dial.getHeight ();
    Matrix matrix = new Matrix ();
    matrix.postScale ((float) scale_width, (float) scale_height);
    bmp_dial = Bitmap.createBitmap (src_bmp_dial, 0, 0, src_bmp_dial.getWidth (), src_bmp_dial.getHeight () , matrix , true);

                                                                        // Create dial
    iv_dial = new ImageView (context);
    RelativeLayout.LayoutParams lp_iv_dial = new RelativeLayout.LayoutParams (width, height);//LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
    lp_iv_dial.addRule (RelativeLayout.CENTER_IN_PARENT);
    addView (iv_dial, lp_iv_dial);

    iv_dial.setImageBitmap (bmp_dial);

    gest_det = new GestureDetector (getContext (), this);               // Enable gesture detector
  }
  


    // Public APIs:

  public void listener_set (gui_dia_listener listener) {
    com_uti.logd ("listener: " + listener);
    m_listener = listener;
  }

  public void dial_angle_set (double angle) {                          // Set dial to angle
    com_uti.logd ("angle: " + angle);
    angle = angle % 360;
    Matrix matrix = new Matrix ();
    iv_dial.setScaleType (ScaleType.MATRIX);   
    matrix.postRotate ((float) angle, m_width / 2, m_height / 2);
    iv_dial.setImageMatrix (matrix);
  }
  
    // Internal APIs:

  private double angle_get (double x, double y) {                       // Cartesian x,y to Polar degrees
    //com_uti.logd ("x: " + x + "  y: " + y);
    return (double) -Math.toDegrees (Math.atan2 (x - 0.5, y - 0.5));
  }


  private Bitmap bitmaps_combine (Bitmap bmp1, Bitmap bmp2) {
    com_uti.logd ("bmp1: " + bmp1 + "  bmp2: " + bmp2);
    Bitmap bmOverlay = Bitmap.createBitmap (bmp1.getWidth(), bmp1.getHeight (), bmp1.getConfig ());
    Canvas canvas = new Canvas (bmOverlay);
    canvas.drawBitmap (bmp1, new Matrix (), null);
    canvas.drawBitmap (bmp2, 0, 0, null);
    return bmOverlay;
  }

  private boolean in_center_get (double x, double y) {                  // Return true if within center (power)
    if (x >= 0.3 && x <= 0.7 && y >= 0.3 && y <= 0.7) {                 // If within center boundaries...
      com_uti.logd ("inside center  x: " + x + "  y: " + y);
      return (true);
    }
    else {
      com_uti.logd ("outside center  x: " + x + "  y: " + y);
      return (false);
    }
  }  

  private boolean in_next_get (double x, double y) {                    // Return true if within next
    if (x >= corner_lo && x <= 1.1 && y >= -0.1 && y <= corner_hi) {    // If within next boundaries...
      com_uti.logd ("inside next  x: " + x + "  y: " + y);
      return (true);
    }
    else {
      com_uti.logd ("outside next  x: " + x + "  y: " + y);
      return (false);
    }
  }  

  private boolean in_prev_get (double x, double y) {                    // Return true if within prev
    if (x >= -0.1 && x <= corner_hi && y >= -0.1 && y <= corner_hi) {   // If within prev boundaries...
      com_uti.logd ("inside prev  x: " + x + "  y: " + y);
      return (true);
    }
    else {
      com_uti.logd ("outside prev  x: " + x + "  y: " + y);
      return (false);
    }
  }  

  private boolean angle_set (double x, double y) {                      //
    double angle = angle_get (1 - x, 1 - y);                            // 1- to correct our custom axis direction
    if (Double.isNaN (angle))                                           // If angle not a valid number...
      return (false);                                                   // Event NOT consumed

    //if (angle < 210 && angle > 150)                                   // Deny full rotation, start start and stop point, and get a linear scale
    //  return (false);                                                 // Event NOT consumed

    boolean consumed = false;
    if (m_listener != null)
      consumed = m_listener.dial_chngd (angle);

    if (consumed)
      dial_angle_set (angle);                                             // Rotate

    return (consumed);//true);                                                      // Event consumed
  }

    // For android.view.GestureDetector.OnGestureListener:

  public boolean onTouchEvent (MotionEvent motion_event) {
    //com_uti.logd ("motion_event: " + motion_event);

    getParent().requestDisallowInterceptTouchEvent (true);              // Don't do horizontal scroll

    boolean have_gest_det = gest_det.onTouchEvent (motion_event);
    //com_uti.logd ("motion_event: " + motion_event + "  have_gest_det: " + have_gest_det);
    if (have_gest_det) {
      com_uti.logd ("motion_event: " + motion_event + "  have_gest_det: " + have_gest_det);
      return (true);                                                    // Event consumed
    }

    if (is_down_dial) {
      double x = x_motion_get (motion_event);
      double y = y_motion_get (motion_event);
      //com_uti.logd ("motion_event: " + motion_event + "  x: " + x + "  y: " + y);

      if (in_center_get (x, y)) {                                       // If within center boundaries...
        //if (m_listener != null)
        //  consumed = m_listener.state_chngd ();                                  // onSingleTapUp handles
        return (true);                                                  // Event consumed
      }
      boolean consumed = motion_angle_set (motion_event);
      if (consumed)
        return (consumed);
    }

    boolean super_ret = super.onTouchEvent (motion_event);
    //com_uti.logd ("super_ret: " + super_ret);
    com_uti.logd ("motion_event: " + motion_event + "  have_gest_det: " + have_gest_det + "  super_ret: " + super_ret);
    return (super_ret);                                                 // Event consumed or NOT consumed as per super

    //return (false);                                                   // Event NOT consumed
  }

  //private state_toggle

  private double x_motion_get (MotionEvent motion_event) {
    double x = motion_event.getX () / ((double) getWidth ());
    return (x);
  }
  private double y_motion_get (MotionEvent motion_event) {
    double y = motion_event.getY () / ((double) getHeight ());
    return (y);
  }

  public boolean onDown (MotionEvent motion_event) {                    // Called when tap starts/down
    //com_uti.logd ("motion_event: " + motion_event);
    double x = x_motion_get (motion_event);
    double y = y_motion_get (motion_event);
    //angle_down = angle_get (1 - x, 1 - y);                            // 1- to correct our custom axis direction
    com_uti.logd ("motion_event: " + motion_event + "  x: " + x + "  y: " + y);

    if (in_center_get (x, y)) {                                       // If within center boundaries for power... let up handle
      is_down_powr = true;
      return (true);                                                    // Event consumed
    }
    else if (in_next_get (x, y)) {
      is_down_next = true;
      return (true);                                                    // Event consumed
    }
    else if (in_prev_get (x, y)) {
      is_down_prev = true;
      return (true);                                                    // Event consumed
    }
    else {
      is_down_dial = true;
      boolean consumed = angle_set (x, y);
      if (consumed)
        return (consumed);
    }
 
    return (false);                                                     // Event NOT consumed
  }
  //private double             angle_down, angle_up;
  public boolean onSingleTapUp (MotionEvent motion_event) {             // Called when tap ends/up
    com_uti.logd ("motion_event: " + motion_event);
    double x = x_motion_get (motion_event);
    double y = y_motion_get (motion_event);
    //angle_up = angle_get (1 - x, 1 - y);                              // 1- to correct our custom axis direction
    com_uti.logd ("x: " + x + "  y: " + y);

    boolean consumed = false;   
    //if (! Float.isNaN (angle_down) && ! Float.isNaN (angle_up) && Math.abs (angle_up-angle_down) < 10) {  // If click up where we clicked down it's just a button press
    if (is_down_dial) {
      is_down_dial = false;
      com_uti.logd ("Finish freq_set");
      if (m_listener != null)
        consumed = m_listener.freq_go ();
      return (consumed);//true);                                                    // Event consumed
    }
    else if (is_down_next) {
      is_down_next = false;
      if (m_listener != null)
        consumed = m_listener.next_go ();
      return (consumed);//true);                                                    // Event consumed
    }
    else if (is_down_prev) {
      is_down_prev = false;
      if (m_listener != null)
        consumed = m_listener.prev_go ();
      return (consumed);//true);                                                    // Event consumed
    }
    else if (is_down_powr) {//in_center_get (x, y)) {                                    // If within center boundaries...
      is_down_powr = false;
      if (m_listener != null)
        consumed = m_listener.state_chngd ();
      return (consumed);//true);                                                    // Event consumed
    }

    return (consumed);//false);                                                     // Event NOT consumed
  }

  private boolean motion_angle_set (MotionEvent motion_event) {
    double x = x_motion_get (motion_event);
    double y = y_motion_get (motion_event);
    boolean consumed = angle_set (x, y);
    return (consumed);
  }
  public boolean onScroll (MotionEvent motion_event1, MotionEvent motion_event2, float dist_x, float dist_y) {
    com_uti.logd ("motion_event1: " + motion_event1 + "motion_event2: " + motion_event2 + "  dist_x: " + dist_x + "  dist_y: " + dist_y);
    boolean consumed = motion_angle_set (motion_event2);
    return (consumed);
  }

  public void onShowPress (MotionEvent motion_event) {
    com_uti.logd ("motion_event: " + motion_event);
  }
  public boolean onFling (MotionEvent motion_event1, MotionEvent motion_event2, float fling1, float fling2) {
//    com_uti.logd ("motion_event1: " + motion_event1 + "motion_event2: " + motion_event2 + "  fling1: " + fling1 + "  fling2: " + fling2);
    return (false);                                                     // Event NOT consumed
  }
  public void onLongPress (MotionEvent motion_event) {
    com_uti.logd ("motion_event: " + motion_event);
  }

/* Don't need:
  private double xDistance, yDistance, lastX, lastY;
  @Override
  public boolean onInterceptTouchEvent (MotionEvent motion_event) {
    com_uti.logd ("motion_event: " + motion_event);

    boolean ret = super.onInterceptTouchEvent  (motion_event);
    if (ret) {

      final HorizontalScrollView scrollView = (HorizontalScrollView) findViewById (R.id.hsv);
      if (scrollView != null)
        scrollView.requestDisallowInterceptTouchEvent (true);

      getParent ().requestDisallowInterceptTouchEvent (true);

    }
    return ret;
  }
*/
//return (false);
/*
    switch (motion_event.getAction ()) {
        case MotionEvent.ACTION_DOWN:
            xDistance = yDistance = 0f;
            lastX = motion_event.getX ();
            lastY = motion_event.getY ();
            break;
        case MotionEvent.ACTION_MOVE:
            final double curX = motion_event.getX ();
            final double curY = motion_event.getY ();
            xDistance += Math.abs(curX - lastX);
            yDistance += Math.abs(curY - lastY);
            lastX = curX;
            lastY = curY;
            if (xDistance > yDistance)                                  // If more horizontal than vertical...
                return (false);                                         // Event NOT consumed
    }

    return super.onInterceptTouchEvent (motion_event);
  }
*/

/*  private Bitmap bmp_dial_on, bmp_dial_off;

From constructor:

    Bitmap src_bmp_off = BitmapFactory.decodeResource (context.getResources (), top_id_off);
    Bitmap src_bmp_off_a = bitmaps_combine (src_bmp_dial, src_bmp_off);
    double off_scale_width = ((double) width) / src_bmp_off_a.getWidth();
    double off_scale_height = ((double) height) / src_bmp_off_a.getHeight();
    Matrix off_matrix = new Matrix ();
    off_matrix.postScale (off_scale_width, off_scale_height);
    bmp_dial_off = Bitmap.createBitmap (src_bmp_off_a, 0, 0, src_bmp_off_a.getWidth(), src_bmp_off_a.getHeight() , off_matrix , true);

    Bitmap src_bmp_on = BitmapFactory.decodeResource (context.getResources (), top_id_off);
    Bitmap src_bmp_on_a  = bitmaps_combine (src_bmp_dial, src_bmp_on);
    double on_scale_width = ((double) width) / src_bmp_on_a.getWidth();
    double on_scale_height = ((double) height) / src_bmp_on_a.getHeight();
    Matrix on_matrix = new Matrix ();
    on_matrix.postScale (on_scale_width, on_scale_height);
    bmp_dial_on = Bitmap.createBitmap (src_bmp_on_a, 0, 0, src_bmp_on_a.getWidth(), src_bmp_on_a.getHeight() , on_matrix , true);
*/

/*  From onTouchEvent:
    if (motion_event.getAction () == MotionEvent.ACTION_DOWN || motion_event.getAction () == MotionEvent.ACTION_MOVE) {
      final HorizontalScrollView scrollView = (HorizontalScrollView) findViewById (R.id.hsv);
      if (scrollView != null)
        scrollView.requestDisallowInterceptTouchEvent (true);

      //return (false);                                                 // Event NOT consumed
    }
*/
/*
      final HorizontalScrollView scrollView = (HorizontalScrollView) findViewById (R.id.hsv);
      if (scrollView != null)
        scrollView.requestDisallowInterceptTouchEvent (true);
*/
/*
    boolean ret = false;
    ret = super.onTouchEvent (motion_event);
    //if (ret)
      getParent().requestDisallowInterceptTouchEvent (true);
    return ret;
*/

}
