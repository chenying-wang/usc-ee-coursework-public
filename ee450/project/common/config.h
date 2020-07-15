#ifndef __EE450_CONFIG
#define __EE450_CONFIG

#define BUFFER_SIZE (0xff)
#define MAX_QUEUED_REQUESTS (0x1ff)
#define MAX_EVENTS (0x1ff)
#define WORKER_STACK_SIZE (0x200000)

#define HEALTHCENTER_SERV_ADDR (htonl(INADDR_LOOPBACK))
#define HEALTHCENTER_SERV_PORT (htons(6000 + USC_ID))

#define HOSPITAL_SERV_ADDR (htonl(INADDR_LOOPBACK))
#define STUDENT_SERV_ADDR (htonl(INADDR_LOOPBACK))

#define MSG_DELIMITER ('\n')

#define PHASE_1 (0x01)
#define PHASE_2 (0x10)
#define TERMINATED (0x00)

#endif
