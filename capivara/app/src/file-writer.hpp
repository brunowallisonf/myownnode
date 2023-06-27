#include<v8.h>
#include<uv.h>


uv_loop_t * writer_default_loop;

static uv_fs_t writerOpenReq;
static uv_fs_t writeReq;
static uv_fs_t writerCloseReq;
static uv_buf_t uvBuffer;
static char strBuffer[65];
static char dataBuffer[64];

struct Writer {
    v8::Global<v8::Function> callback;
    v8::Local<v8::Context>  * context;
    v8::Isolate * isolate;
    v8::Local<v8::Value> dataToWrite;
};

Writer * writer = new Writer();


class FileWriter {
    public:
         static void Initialize(uv_loop_t *evloop) {
            writer_default_loop = evloop;
        }


    static void handleCallback(char * error, char* data) {
        v8::Local<v8::Function> callback = v8::Local<v8::Function>::New(writer->isolate, writer->callback);

        v8::Local<v8::Value> result;
          v8::Handle<v8::Value> resultr [] = { v8::Undefined(writer->isolate) , v8::Undefined(writer->isolate) };


        if(error != NULL){
            resultr[0] = v8_str(error);
        }else{
            resultr[1] = v8_str(data);
        }

        if(callback->Call(*(writer->context),v8::Undefined(writer->isolate),2,resultr).ToLocal(&result)){

        }else{
                
        }

    }

    static void onWrite(uv_fs_t *req)
    {
        
        if (req->result < 0) {
            const char * error = uv_strerror(req->result);
            fprintf(stderr, "error: %s\n", error );
            handleCallback((char*)error,NULL);
        } else if (req->result == 0) {
            uv_fs_close(writer_default_loop, &writerCloseReq, writerOpenReq.result, NULL);
        } else {
            //handleCallback(NULL,NULL);

        }
    }

    static void onOpen(uv_fs_t *req)
    {
        
        if (req->result < 0) {
            const char * error = uv_strerror(req->result);
            //fprintf(stderr, "error: %s\n", error );
            handleCallback((char*)error,NULL);
        } else {
            v8::String::Utf8Value str(writer->isolate,writer->dataToWrite);
            std::string cppStr(*str);
            char *buf = new char[cppStr.length() + 1];
            strcpy(buf, cppStr.c_str());
                
            uv_buf_t iov = uv_buf_init(buf, sizeof(buf));
            iov.len = req->result;

            uv_fs_write(writer_default_loop, &writeReq, req->result, &iov, 1 , -1, onWrite);
            printf("%s","here");
        }
        uv_fs_req_cleanup(req);
    }

    static void WriteFile (
        const v8::FunctionCallbackInfo<v8::Value> &args){
            auto isolate = args.GetIsolate();
            auto context = isolate->GetCurrentContext();
            writer->isolate = isolate;
            writer->context = &context;

            v8::String::Utf8Value str(isolate, args[0]);
            writer->dataToWrite = args[1];
            v8::Local<v8::Value> callback = args[2];

            if(!callback->IsFunction()){
                printf("Callback is not a function");
                return ;
            }

            writer->callback.Reset(isolate,callback.As<v8::Function>());
            std::string cppStr(*str);
            char *cstr = new char[cppStr.length() + 1];
            strcpy(cstr, cppStr.c_str());
            uv_fs_open(writer_default_loop, &writerOpenReq, cstr, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR, onOpen);
            uv_run(writer_default_loop, UV_RUN_DEFAULT);
        }
};