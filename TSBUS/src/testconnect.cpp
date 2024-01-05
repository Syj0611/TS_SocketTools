#include "TSCANDef.hpp"


int main(){
    u64 hwhandle;
    initialize_lib_tscan(true, false, false);

    tscan_connect("",&hwhandle);

    finalize_lib_tscan();


    return 0;
}
