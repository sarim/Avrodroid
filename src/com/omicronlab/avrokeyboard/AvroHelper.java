package com.omicronlab.avrokeyboard;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import android.content.Context;

public class AvroHelper {
	public static boolean isFirstLaunch(PhoneticIM Caller, String filename){
		//TODO: Need to check if this is first launch, so should i copy the asset or not.
		//Now i'm just checking if the js file copied, but actually need to check timestamp or something.
		//Bcz if a older version of file exists, i need to detect that using timestamp and copy newer file.
		
		String filepath = Caller.getFilesDir().getAbsolutePath() + "/" + filename;
		return !(new File(filepath).exists());
	}

	public static boolean copyJavascriptLib(PhoneticIM Caller, String fileName){

		try {
			InputStream localInputStream = Caller.getAssets().open(fileName);
	        FileOutputStream localFileOutputStream = Caller.getBaseContext().openFileOutput(fileName, Context.MODE_PRIVATE);
	
	        byte[] arrayOfByte = new byte[1024];
	        int offset;
	        while ((offset = localInputStream.read(arrayOfByte))>0)
	        {
	          localFileOutputStream.write(arrayOfByte, 0, offset);
	        }
	        localFileOutputStream.close();
	        localInputStream.close();
	        return true;
		}
		catch (IOException localIOException)
        {
            localIOException.printStackTrace();
            return false;
        }
	}

}
