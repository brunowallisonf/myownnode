#include<v8.h>
#include<uv.h>


uv_loop_t * default_loop;

static uv_fs_t openReq;
static uv_fs_t readReq;
static uv_fs_t closeReq;
static uv_buf_t uvBuf;
static char strBuf[65];
static char dataBuf[64];

struct Reader {
    v8::Global<v8::Function> callback;
    v8::Local<v8::Context>  * context;
    v8::Isolate * isolate;
};

Reader * reader = new Reader();
class FileReader {
    public:
         static void Initialize(uv_loop_t *evloop) {
            default_loop = evloop;
        }


    static void handleCallback(char * error, char* data) {


        v8::Local<v8::Function> callback = v8::Local<v8::Function>::New(reader->isolate, reader->callback);

        v8::Local<v8::Value> result;
          v8::Handle<v8::Value> resultr [] = { v8::Undefined(reader->isolate) , v8::Undefined(reader->isolate) };


        if(error != NULL){
            resultr[0] = v8_str(error);
        }else{
            resultr[1] = v8_str(data);
        }
        

        if(callback->Call(*(reader->context),v8::Undefined(reader->isolate),2,resultr).ToLocal(&result)){

        }else{
                
        }

    }
    static void readData(void)
    {
        memset(strBuf, 0, sizeof(strBuf));
        memcpy(strBuf, dataBuf, sizeof(dataBuf));
        //fprintf(stdout, "got some data: %s\n", strBuf);
        memset(dataBuf, 0, sizeof(dataBuf));
        uv_fs_read(default_loop, &readReq, openReq.result, &uvBuf, 1, -1, onRead);
        handleCallback(NULL,strBuf);
   
    }

    static void onRead(uv_fs_t *req)
    {
        uv_fs_req_cleanup(req);
        if (req->result < 0) {
            const char * error = uv_strerror(req->result);
            //fprintf(stderr, "error: %s\n", error );
            handleCallback((char*)error,NULL);
            
        } else if (req->result == 0) {
            uv_fs_close(default_loop, &closeReq, openReq.result, NULL);
        } else {
            readData();
        }
    }

    static void onOpen(uv_fs_t *req)
    {
        if (req->result < 0) {
            const char * error = uv_strerror(req->result);
            //fprintf(stderr, "error: %s\n", error );
            handleCallback((char*)error,NULL);
        } else {
            uvBuf = uv_buf_init(dataBuf, sizeof(dataBuf));
            uv_fs_read(default_loop, &readReq, req->result, &uvBuf, 1, -1, onRead);
        }
        uv_fs_req_cleanup(req);
    }

    static void ReadFile (
        const v8::FunctionCallbackInfo<v8::Value> &args){
            auto isolate = args.GetIsolate();
            auto context = isolate->GetCurrentContext();
            reader->isolate = isolate;
            reader->context = &context;
            v8::String::Utf8Value str(isolate, args[0]);

            v8::Local<v8::Value> callback = args[1];

            if(!callback->IsFunction()){
                printf("Callback is not a function");
                return ;
            }

            
            reader->callback.Reset(isolate,callback.As<v8::Function>());

            std::string cppStr(*str);
            char *cstr = new char[cppStr.length() + 1];
            strcpy(cstr, cppStr.c_str());
            

            uv_fs_open(default_loop, &openReq, cstr, O_RDONLY, 0, onOpen);
            uv_run(default_loop, UV_RUN_DEFAULT);
        }
};