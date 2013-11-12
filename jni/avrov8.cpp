#include <jni.h>
#include <iostream>
#include <v8.h>
#include <string>
#include <stdio.h>
#include <android/log.h>
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,"avro-ndk",__VA_ARGS__)
#define  LOGIJS(...)  __android_log_print(ANDROID_LOG_INFO,"avro-js",__VA_ARGS__)

using namespace v8;

HandleScope handle_scope;

std::string libdir;

// Create a new context.
Persistent<Context> context;
	
Handle<String> ReadFile(const char* name) {
  FILE* file = fopen(name, "rb");
  if (file == NULL) return Handle<String>();

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  char* chars = new char[size + 1];
  chars[size] = '\0';
  for (int i = 0; i < size;) {
    int read = fread(&chars[i], 1, size - i, file);
    i += read;
  }
  fclose(file);
  Handle<String> result = String::New(chars, size);
  delete[] chars;
  return result;
}


const char* ToCString(const String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

Handle<Value> Print(const Arguments& args) {
  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    HandleScope handle_scope;
    if (first) {
      first = false;
    } else {
      //printf(" ");
    }
    String::Utf8Value str(args[i]);
    const char* cstr = ToCString(str);
    LOGIJS("%s", cstr);
  }
  return Undefined();
}


void loadfile(std::string filename){
    Context::Scope context_scope(context);
	LOGI("Loading js file : %s",filename.c_str());

    std::string filepath = libdir + filename;
    // Create a string containing the JavaScript source code.
    Handle<String> source = ReadFile(filepath.c_str());
    // Compile the source code.
    Handle<Script> script = Script::Compile(source);
	
	script->Run();
	LOGI("Finished Loding file.");
	
}

extern "C" {
	JNIEXPORT void JNICALL Java_com_omicronlab_avrokeyboard_PhoneticIM_setdir( JNIEnv* env,jobject thiz, jstring sk1);
	JNIEXPORT void JNICALL Java_com_omicronlab_avrokeyboard_PhoneticIM_loadjs( JNIEnv* env,jobject thiz, jstring sk1, jstring sk2);
	JNIEXPORT void JNICALL Java_com_omicronlab_avrokeyboard_PhoneticIM_killjs( JNIEnv* env,jobject thiz);
	JNIEXPORT jstring JNICALL Java_com_omicronlab_avrokeyboard_PhoneticIM_avroparse( JNIEnv* env,jobject thiz,jstring sk1);
};

void GetJStringContent(JNIEnv *AEnv, jstring AStr, std::string &ARes) {
  if (!AStr) {
    ARes.clear();
    return;
  }

  const char *s = AEnv->GetStringUTFChars(AStr,NULL);
  ARes=s;
  AEnv->ReleaseStringUTFChars(AStr,s);
}


void Java_com_omicronlab_avrokeyboard_PhoneticIM_loadjs(JNIEnv* env,jobject thiz,jstring sk1, jstring sk2){
    std::string filepath1;
    std::string filepath2;
    
    //convert java String to Cpp String
    GetJStringContent(env,sk1,filepath1);
    GetJStringContent(env,sk2,filepath2);
    
    
    // Enter the created context for compiling and running script. 

	Handle<ObjectTemplate> global = ObjectTemplate::New();
	
	global->Set(String::New("print"), FunctionTemplate::New(Print));
	
	context = Context::New(NULL, global);
	
	loadfile(filepath1);
	loadfile(filepath2);
	
}

void Java_com_omicronlab_avrokeyboard_PhoneticIM_killjs( JNIEnv* env,jobject thiz){
	
	Context::Scope context_scope(context);
	context.Dispose();
	
}

void Java_com_omicronlab_avrokeyboard_PhoneticIM_setdir( JNIEnv* env,jobject thiz, jstring sk1){
    GetJStringContent(env,sk1,libdir);
    LOGI("Setting js files dir path : %s",libdir.c_str());
}

jstring JNICALL Java_com_omicronlab_avrokeyboard_PhoneticIM_avroparse( JNIEnv* env,jobject thiz,jstring sk1) {
	
  std::string banglatxt;

  //convert java String to Cpp String  
  GetJStringContent(env,sk1,banglatxt);
    
  Context::Scope context_scope(context);
  
  Handle<Object> global = context->Global();
  Handle<Value> avro_parse = global->Get(String::New("avroparsefunc"));
  Handle<Function> avro_parse_func = Handle<Function>::Cast(avro_parse);
  Handle<Value> args[1];
  Handle<Value> result;

  args[0] = v8::String::New(banglatxt.c_str());
  
  result = avro_parse_func->Call(global, 1, args);
  

  // Convert the result to an ASCII string 
  String::Utf8Value ascii(result);

  std::string bntext = std::string(*ascii);
  
  return env->NewStringUTF(bntext.c_str());
}
