//GDI8001 Plugin header

struct typeofpluginctx {
    UINT32 version;
    BOOL ispluginloaded;
    BOOL isexecutedontheemulator;
    UINT32 plugintype[24];
    int(*ptrofz80memaccess)(int,int,int);
    int(*uniquememaccess)(int, int, int);
    void (*Z80INT)(UINT8);
    bool isvalidport;
};

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) void InitPlugin(typeofpluginctx*);
#ifdef __cplusplus
}
#endif
