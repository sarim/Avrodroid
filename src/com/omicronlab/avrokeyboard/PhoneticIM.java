package com.omicronlab.avrokeyboard;

import java.util.ArrayList;
import java.util.List;
import java.util.logging.Logger;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.content.Context;
import android.inputmethodservice.InputMethodService;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.Keyboard.*;
import android.inputmethodservice.KeyboardView;
import android.inputmethodservice.KeyboardView.OnKeyboardActionListener;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.widget.LinearLayout.LayoutParams;

public class PhoneticIM extends InputMethodService implements OnKeyboardActionListener {
	String ctext = "";
	CandidateView mCandidateView;
	@Override
	public void onCreate() {
        super.onCreate();
        Log.v("Avro","Avro started");
        
        String filename1 = "avro.min.js" ;
        String filename2 = "avro.init.js" ;
        
        if (AvroHelper.isFirstLaunch(this, filename1)){
        	boolean success = AvroHelper.copyJavascriptLib(this,filename1) && AvroHelper.copyJavascriptLib(this,filename2);
        	Log.v("Avro","Coping JS File " + (success ? "Sucessful" : "Failed") );
        	
        } else {
        	Log.v("Avro","JS File Already Copied, Not copying again");
        }
        // TODO: Need to crash the app now if js file copy failed
        
        // Load the js file inside v8
        setdir(getFilesDir().getAbsolutePath() + "/");
        long startTime = System.nanoTime();
        loadjs(filename1, filename2);
        long endTime = System.nanoTime();

        double duration = (endTime - startTime) / 1000000000.0;
        Log.v("Avro","Js Load Time: " + duration);
        
    }
	
	@Override 
	public View onCreateInputView() {
		KeyboardView kbdview = (KeyboardView) getLayoutInflater().inflate(
                R.layout.input, null);
		kbdview.setOnKeyboardActionListener(this);
		
		kbdview.setKeyboard(new Keyboard(this, R.xml.qwerty));

		return kbdview;
	}
	
	@Override 
	public View onCreateCandidatesView() {
		mCandidateView = new CandidateView(this);
		mCandidateView.setService(this);
        return mCandidateView;
		
	}
	
	@Override 
	public void onStartInputView(EditorInfo attribute, boolean restarting) {
        super.onStartInputView(attribute, restarting);
	}
	
	@Override 
	public void onFinishInput() {
        super.onFinishInput();
	}

	@Override
	public void onKey(int arg0, int[] arg1) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onPress(int arg0) {
		
		// TODO Need To handle all keycodes here
		
		Log.v("Avro", "Code = " + arg0);

		InputConnection ic = getCurrentInputConnection();

		if ( arg0 >= 32)		
		ctext += (char) arg0;
		else if (arg0 == -5) //handle backspace
		{
			if (ctext.length() > 0) 
				ctext = ctext.substring(0, ctext.length() - 1);
		}
		
		if(arg0 == 32) {
			ic.commitText(ctext, 1);
			ctext = "";
			return;
		}
		
		
		ic.setComposingText(ctext, 1);
		//setCandidatesView(mCandidateView);
		
		List<String> suggestions = SuggestionBuilder(ctext);
		
		
		mCandidateView.setSuggestions(suggestions , true, true);
		setCandidatesViewShown(true);
	}

	@Override
	public void onRelease(int arg0) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onText(CharSequence arg0) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void swipeDown() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void swipeLeft() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void swipeRight() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void swipeUp() {
		// TODO Auto-generated method stub
		
	}
	
	List<String> SuggestionBuilder(String entext) {
		List<String> list = new ArrayList<String>();
		
		String bntext = avroparse(entext);
		try {
			
			JSONArray words = new JSONArray(bntext);
			
			for (int index = 0 ; index < words.length() ; ++index){
				list.add(words.get(index).toString());
			}
			
		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return list;
	}

	
	
	public native void  setdir(String dirname);
	public native void  loadjs(String sk1, String sk2);
	public native void  killjs();
	public native String avroparse(String sk1); 
	static {
		System.loadLibrary("stlport_shared");
		System.loadLibrary("v8");
        System.loadLibrary("avrov8");
    }
}