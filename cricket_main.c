#include "cap_proc.h"

int main(){
  pthread_t cap_thread, proc_thread;
  
  init_globes();

  open_device();
  init_device();
  start_streaming();
  capturing_thread(&cap_thread);
  
  processing_thread(&proc_thread);

  monitoring_window();

  pthread_join(cap_thread, NULL);
  stop_streaming();
  uninit_device();
  close_device();
  return 0;
}
