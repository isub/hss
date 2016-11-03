#include <freeDiameter/extension.h>
#include <signal.h>

#include "app_s6a.h"

/* обработка сигналов */
static void sighandler_cb (void);

int signal_handler_init ()
{
  CHECK_FCT (fd_event_trig_regcb (SIGUSR1, "app_s6a", sighandler_cb));

  return 0;
}

void sighandler_cb (void)
{
  CHECK_FCT_DO (app_s6a_client_send_air (NULL), return;);
}
