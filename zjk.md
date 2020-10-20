1. 编译时这两个文件找不到：
#include <net/if_vlan_var.h>
#include <net/if_mib.h>
2. 使用说明编译也是同样的错误 make -f Makefile.standalone
3. apt-file search 均找不到
4. www也没有
5. Makefile.base中有.include <bsd.lib.mk>
可能是bsd中的头文件

