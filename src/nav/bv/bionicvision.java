package nav.bv;

import java.io.IOException;

import android.app.Activity;
import android.graphics.Canvas;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.Size;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.RadioButton;
import android.widget.RelativeLayout;

public class bionicvision extends Activity {
	private CustomView cv;
	Camera camera = Camera.open();
	static {System.loadLibrary("image_proc");}
	public native void initializer(int threshold, int binary_threshold);
	
	int threshold;
	int tag;
	
	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		setContentView(R.layout.main);
		cv = new CustomView(this.getApplicationContext());
		final RelativeLayout main = (RelativeLayout) this.findViewById(R.id.mainlayout);
		main.addView(cv);

		try 
		{
			camera.setPreviewDisplay(dummySurfaceHolder);
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}

		Parameters params = camera.getParameters();
		params.setPreviewSize(800, 480); // not all platforms may support this size, should do a search for closest match
		params.set("iso", "800");
		camera.setParameters(params);

		final Size size = params.getPreviewSize();
		final int bpp = ImageFormat.getBitsPerPixel(params.getPreviewFormat());
		final int frameSize = (size.width * size.height * bpp) / 8;

		camera.setPreviewCallbackWithBuffer(cv.previewCallback);
		camera.addCallbackBuffer(new byte[frameSize]);
		camera.startPreview();
		
		final RadioButton radio16 = (RadioButton) findViewById(R.id.radio16);
		radio16.setOnClickListener(grayscalelevel_listener);
		final RadioButton radio8 = (RadioButton) findViewById(R.id.radio8);
		radio8.setOnClickListener(grayscalelevel_listener);
		final RadioButton radio4 = (RadioButton) findViewById(R.id.radio4);
		radio4.setOnClickListener(grayscalelevel_listener);
		final RadioButton radio2 = (RadioButton) findViewById(R.id.radio2);
		radio2.setOnClickListener(grayscalelevel_listener);
		final RadioButton radioAdaptive = (RadioButton) findViewById(R.id.adaptive_threshold);
		radioAdaptive.setOnClickListener(threshold_listener);

	}
	
	public void onPause()
	{
		super.onPause();
		camera.release();
	}

	private OnClickListener grayscalelevel_listener = new OnClickListener() {	//do this when threshold button is pressed
		public void onClick(View v)
		{
			RadioButton rb = (RadioButton) v;
			int label = Integer.parseInt(rb.getText().toString());
			threshold = label;
			initializer(threshold,0);
		}
	};
	private OnClickListener threshold_listener = new OnClickListener() {	//do this when threshold button is pressed
		public void onClick(View v)
		{
			RadioButton rb = (RadioButton) v;
			threshold = 2;
			tag = Integer.parseInt(rb.getTag().toString());
			initializer(threshold,tag);
		}
	};
	
	private SurfaceHolder dummySurfaceHolder = new SurfaceHolder() 
	{

		@Override
		public void unlockCanvasAndPost(Canvas canvas)
		{
		}

		@Override
		public void setType(int type)
		{
		}

		@Override
		public void setSizeFromLayout()
		{
		}

		@Override
		public void setKeepScreenOn(boolean screenOn)
		{
		}

		@Override
		public void setFormat(int format) 
		{
		}

		@Override
		public void setFixedSize(int width, int height)
		{
		}

		@Override
		public void removeCallback(Callback callback)
		{
		}

		@Override
		public Canvas lockCanvas(Rect dirty)
		{
			return null;
		}

		@Override
		public Canvas lockCanvas()
		{
			return null;
		}

		@Override
		public boolean isCreating()
		{
			return false;
		}

		@Override
		public Rect getSurfaceFrame()
		{
			return null;
		}

		@Override
		public Surface getSurface()
		{
			return null;
		}

		@Override
		public void addCallback(Callback callback)
		{
		}
	};

}