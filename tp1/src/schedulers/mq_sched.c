#include <sched.h>

#define STATE_RUNNING   0
#define STATE_BLOCKED   1
#define STATE_FINISHED  2

#define TYPE_NONE      -1
#define TYPE_REALTIME   0
#define TYPE_LOW        1

#define LOW_QUANTUM     2
#define RT_QUANTUM      4

char* states[] = {"running", "blocked", "finished"};

typedef struct str_sched_task {
  int state;
  int type;
  pid next;
  pid prev;
} sched_task;


sched_task tasks[MAX_PID];

static uint_32 quantum;
static pid current_pid;
static pid next_pid[2];


void sched_init(void) {
  next_pid[TYPE_REALTIME] = next_pid[TYPE_LOW] = 0; // Cola vacia
  current_pid = 0; // IDLE task
  quantum = LOW_QUANTUM;
}

// Todas las tareas comienzan por ser de baja prioridad
void sched_load(pid pd) {
  enqueue(pd, TYPE_LOW);
}

// Si una tarea se desbloquea, pasa a tener alta prioridad
void sched_unblock(pid pd) {
  enqueue(pd, TYPE_REALTIME);

  // Si esta IDLE salto inmediatamente a la nueva tarea
  // VER SI SE LO AGREGO AL SCHED_LOAD
  if( current_pid == 0){
    current_pid = pd;
    loader_switchto(pd);
  }
}

int sched_exit() {
  dequeue(STATE_FINISHED);
  return update_current_pid();
}

int sched_block() {
  dequeue(STATE_BLOCKED);
  return update_current_pid();
}

/*
 * IDLE task      => devolvemos la siguiente.
 * Task Y Quantum => seguimos corriendo.
 * Fin Quantum    => movemos de cola y ejecutamos la siguiente.
 */
int sched_tick() {
  if (current_pid > 0 && --quantum > 0)
    return current_pid;

  if (quantum == 0) {
    pid tmp_pid = current_pid;
    dequeue(STATE_RUNNING);
    // Se excedio en tiempo de ejecucion, la bajamos.
    enqueue(tmp_pid, TYPE_LOW);
  }

  return update_current_pid();
}

/*
 * Private
 */

void configure_task(pid pd, int type, int state, pid next, pid prev){
  tasks[pd].type  = type;
  tasks[pd].state = state;
  tasks[pd].next  = next;
  tasks[pd].prev  = prev;
}

void enqueue(pid pd, int type) {
  pid next = pd;
  pid prev = pd;

  if (next_pid[type] != 0) {
    next = next_pid[type];
    prev = tasks[next].prev;
  }

  configure_task(pd, type, STATE_RUNNING, next, prev);

  tasks[next].prev = pd;
  tasks[prev].next = pd;

  // Sigue apuntando al mismo next, a menos que este vacia. En ese caso apunta a si misma
  next_pid[type] = tasks[pd].next;
}

// Quita la tarea current_pid de la cola, y le cambia el estado.
pid dequeue(int state) {
  pid next = tasks[current_pid].next;
  pid prev = tasks[current_pid].prev;

  // Es el ultimo elemento en la cola
  if (next == current_pid) {
    next_pid[tasks[current_pid].type] = 0;
  }else{
    tasks[next].prev = prev;
    tasks[prev].next = next;
    next_pid[tasks[current_pid].type] = next;
  }

  // Como se desencola, no pertenece a ninguna cola
  configure_task(current_pid, TYPE_NONE, state, -1, -1);

  return current_pid;
}

// Updates the current_pid based on priorities and set the quantum
pid update_current_pid() {
  quantum = RT_QUANTUM;
  // Pasamos a la proxima tarea de REALTIME
  current_pid = next_pid[TYPE_REALTIME];

  // Avanzamos el puntero al next_pid
  next_pid[TYPE_REALTIME] = tasks[current_pid].next;

  // Si no hay tareas en REALTIME, corremos las de LOW o la IDDLE.
  if (current_pid <= 0) {
    quantum = LOW_QUANTUM;
    current_pid = next_pid[TYPE_LOW];

    // Avanzamos el puntero al next_pid
    next_pid[TYPE_LOW] = tasks[current_pid].next;
  }

  return current_pid;
}

/*
 * END Private
 */

// Show
void showTasks(){
  int i = 0;
  for(i = 0 ; i <= 4 ; i++){
    show_task_structure(i);
  }
  printf("current pid: %d", current_pid);
  printf("next RT: %d | next LOW: %d", next_pid[TYPE_REALTIME], next_pid[TYPE_LOW]);
}


void show_task_structure(pid pd){
  sched_task task = tasks[pd];

  printf("  %d > State: %s | Type: %d | Next: %d | Prev: %d |", pd, states[task.state], task.type, task.next, task.prev);
}


/*
 * Testing
 */

bool sched_test_size(int type, int expected_size){
  int size;
  if (next_pid[type] == 0){
    size = 0;
  }else{
    pid aux = next_pid[type];
    size = 1;
    while(tasks[aux].next != next_pid[type]){
      size++;
      aux = tasks[aux].next;
    }
  }

  if( expected_size != size){
    printf("Size %d, expected %d", size, expected_size);
    hlt();
    return 0;
  }
  return 1;
}

bool sched_test_next_pid(int type, pid expected_pid){
  if( next_pid[type] != expected_pid){
    printf("Next pid[%d] = %d, expected %d", type, next_pid[type], expected_pid);
    hlt();
    return 0;
  }
  return 1;
}

bool sched_test_current(pid expected_current_pid){
  if( current_pid != expected_current_pid){
    printf("Current pid = %d, expected %d", current_pid, expected_current_pid);
    hlt();
    return 0;
  }
  return 1;
}

bool sched_test_status(pid task_pid, int expected_state){
  if (tasks[task_pid].state != expected_state){
    printf("State of %d is %d, expected %d", task_pid, tasks[task_pid].state, expected_state);
    hlt();
    return 0;
  }
  return 1;
}

bool sched_test_type(pid task_pid, int expected_type){
  if (tasks[task_pid].type != expected_type){
    printf("Type of %d is %d, expected %d", task_pid, tasks[task_pid].type, expected_type);
    hlt();
    return 0;
  }
  return 1;
}

bool sched_test_node(pid node, pid expected_next, pid expected_prev){
  if((tasks[node].next != expected_next)||(tasks[node].prev != expected_prev)){
    printf("Node %d [next]=%d, [prev]=%d, expected [next]=%d, expected [prev]=%d",node, tasks[node].next, tasks[node].prev, expected_next, expected_prev);
    hlt();
    return 0;
  }
  return 1;
}
