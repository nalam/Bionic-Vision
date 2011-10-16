package nav.bv;

import android.content.Context;
import android.graphics.Canvas;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.view.View;
//import android.util.TimingLogger;

public class CustomView extends View
{
	static {System.loadLibrary("image_proc");}
	public native int[] meanArray(int res_w, int res_h, byte[] frame);
	public native int[] meanArrayAdapt(int res_w, int res_h, byte[] frame);
	//public native double[] fillmask(int res_w, int res_h);
	public native int[] pixelArray(int[] meanArr, int res_w, int res_h);
	public native void initializer(int threshold);
	public native int getAdaptive();
	
	public Camera camera;
	
	private final int width = 800;
	private final int height = 480;	//height and width of display
	private int[] pixels = new int[width * height];	//array to store the processed frame
	private int[] meanArr;
	private int res = 25;	//resolution being downsampled to
	private int crop_w = width-400;
	private int crop_h = height-80;	//height and width of cropped image
	private int res_h = crop_h/res;
	private int res_w = crop_w/res;
	//private double[] multi_mask = new double[(res_h)*(res_w)];	//array to hold the gaussian distribution numbers

	public CustomView(Context context) 
	{
		super(context);
		//System.arraycopy(fillmask(res_w, res_h), 0, multi_mask, 0, res_w*res_h);
		initializer(16);
	}

	public void onDraw(Canvas canvas) 
	{
		//TimingLogger timings1 = new TimingLogger("TAG", "methodB");
		canvas.drawBitmap(pixels, 0, crop_w, 200, 40, crop_h, crop_w, false, null);	//draw the pixel array onto the canvas
		//timings1.addSplit("workB");
		//timings1.dumpToLog();
	}

	public PreviewCallback previewCallback = new PreviewCallback()
	{
		@Override
		public void onPreviewFrame(byte[] frame, Camera c)	//where the image processing takes place
		{
			//TimingLogger timings = new TimingLogger("TAG", "methodA");
			int check = getAdaptive();	//check to see if adaptive threshold is set
			if(check == 0)
			{ 
				meanArr = meanArray(res_w,res_h,frame);
			}
			else
			{ 
				meanArr = meanArrayAdapt(15,15,frame); //tweaked method to find mean and laplacian edges
			}
			
			//timings.addSplit("workA");
				
			pixels = pixelArray(meanArr, res_w, res_h);
			//timings.addSplit("workB");
			
			//timings.dumpToLog();
			c.addCallbackBuffer(frame);
			CustomView.this.invalidate();
		}
	};
}