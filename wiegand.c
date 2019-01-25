#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <jni.h>
#include <string.h>
#include <strings.h>

#include<android/log.h>

#define WG_CMD_MAGIC 		'x'
#define WG_26_MODE			_IO(WG_CMD_MAGIC, 0x01)
#define WG_34_MODE			_IO(WG_CMD_MAGIC, 0x02)
#define WG_66_MODE			_IO(WG_CMD_MAGIC, 0x03)
#define WG_PERIOD_MODE		_IO(WG_CMD_MAGIC, 0x04)
#define WG_PULSE_MODE		_IO(WG_CMD_MAGIC, 0x05)
#define WG_UNKNOWN_MODE 	_IO(WG_CMD_MAGIC, 0x07)

#define TAG "wiegand-jni"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)

unsigned long wiegand_read(void)
{
	int fd;
	int err;
	unsigned long read_id;
	fd_set readset;
	
	FD_ZERO(&readset);
	
	fd = open("/dev/wiegand", O_RDWR);
	if (fd < 0)	{
		LOGE("can't open!");
		return -1;
	}

	FD_SET(fd, &readset);
	//if(!fork()){
		while(1){
			int err = select(fd+1, &readset, NULL, NULL, NULL);
			if (err < 0) {
				LOGE("select error");
			} else if (err == 0) {
				continue;
			}

			if (FD_ISSET(fd, &readset)) {
				read(fd, &read_id, sizeof(read_id));
				LOGI("read_id = %lu",read_id);
				close(fd);
				return read_id;
			}
			close(fd);
			return -1;
		}
	//}
}

int wiegand_write(JNIEnv* env, char cmd, jcharArray buf)
{
	int fd;
	int ret = -1;
	char t_cmd = cmd;
	size_t datalen = 0;
	jchar* array = (*env)->GetCharArrayElements(env, buf, NULL);
	uint32_t arraysize = (*env)->GetArrayLength(env, buf);
	char *data;
	uint32_t i = 0;
	
	datalen = arraysize + 1;//第0位为命令字，整个数据长度加一
	
	data = (char *)calloc(datalen, sizeof(char));
	
	LOGI("arraysize is %d", arraysize);
	
	data[0] = t_cmd;
	
	for(;i < arraysize; i++){
		data[i+1] = array[i];
	}
	data[arraysize+1] = '\0';
	
	for (i = 0; i < datalen; i++){
		LOGI("data[%d] is 0x%x",i, data[i]);
	}
	
	fd = open("/dev/wiegand", O_RDWR);
	if (fd < 0)	{
		LOGE("can't open!");
		return -1;
	}
	
	(*env)->ReleaseCharArrayElements(env, buf, array, 0);
	LOGI("t_cmd is 0x%x",t_cmd);
	switch(t_cmd){
		case 0x01: //wg_26
			ret = ioctl(fd, WG_26_MODE, data+1);
			break;
		case 0x02: //wg_34
			ret = ioctl(fd, WG_34_MODE, data+1);
			break;
		case 0x03: //wg_66
			ret = ioctl(fd, WG_66_MODE, data+1);
			break;
		case 0x04: //period
			ret = ioctl(fd, WG_PERIOD_MODE, data+1);
			break;
		case 0x05: //pluse
			ret = ioctl(fd, WG_PULSE_MODE, data+1);
			break;
	}
	//ret = write(fd, data, datalen);	
	
	LOGD("ret = %d", ret);
	
	close(fd);
	
	return ret;
}	
	
//MainActivity中调用的add函数，作用就是连接java层和C/C++层的API，格式是java+包名+类名+函数名
JNIEXPORT jint JNICALL
Java_wiegand_Wiegand_wiegandWrite(JNIEnv* env, jobject jobj, jchar cmd, const jcharArray buf){
    return wiegand_write(env, cmd, buf);
}

JNIEXPORT jlong JNICALL
Java_wiegand_Wiegand_wiegandRead(JNIEnv* env, jobject jobj){
    return wiegand_read();
}
