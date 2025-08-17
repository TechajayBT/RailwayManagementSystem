#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TRAINS_FILE "trains.dat"
#define TICKETS_FILE "tickets.dat"

// ===================== Data Models =====================
typedef struct {
    int train_no;
    char name[50];
    char source[30];
    char destination[30];
    int seats_total;
    int seats_available;
    float fare;
} Train;

typedef struct {
    int ticket_id;
    char name[50];
    int age;
    int train_no;
    int seat_no; 
} Passenger;

// ===================== Utilities =====================
static void pause_enter(void){
    printf("\nPress ENTER to continue...");
    int c; while ((c = getchar()) != '\n' && c != EOF) {}
}

static int read_int(const char *prompt){
    int x; char line[128];
    for(;;){
        if (prompt) printf("%s", prompt);
        if (!fgets(line, sizeof(line), stdin)) return 0;
        if (sscanf(line, "%d", &x) == 1) return x;
        printf("Invalid number. Try again.\n");
    }
}

static float read_float(const char *prompt){
    float x; char line[128];
    for(;;){
        if (prompt) printf("%s", prompt);
        if (!fgets(line, sizeof(line), stdin)) return 0.0f;
        if (sscanf(line, "%f", &x) == 1) return x;
        printf("Invalid number. Try again.\n");
    }
}

static void read_line(const char *prompt, char *out, size_t cap){
    if (prompt) printf("%s", prompt);
    if (fgets(out, (int)cap, stdin)){
        size_t n = strlen(out);
        if (n && out[n-1] == '\n') out[n-1] = '\0';
    } else if (cap) {
        out[0] = '\0';
    }
}

// ===================== File Helpers =====================
static long train_record_count(){
    FILE *fp = fopen(TRAINS_FILE, "rb");
    if(!fp) return 0;
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fclose(fp);
    return sz / (long)sizeof(Train);
}

static long ticket_record_count(){
    FILE *fp = fopen(TICKETS_FILE, "rb");
    if(!fp) return 0;
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fclose(fp);
    return sz / (long)sizeof(Passenger);
}

static int find_train(int train_no, Train *out, long *index){
    FILE *fp = fopen(TRAINS_FILE, "rb");
    if(!fp) return 0;
    Train t; long i = 0;
    while (fread(&t, sizeof(Train), 1, fp) == 1){
        if (t.train_no == train_no){
            if (out) *out = t;
            if (index) *index = i;
            fclose(fp);
            return 1;
        }
        i++;
    }
    fclose(fp);
    return 0;
}

static int write_train_at(long index, const Train *t){
    FILE *fp = fopen(TRAINS_FILE, "rb+");
    if(!fp) return 0;
    if (fseek(fp, index * (long)sizeof(Train), SEEK_SET) != 0){ fclose(fp); return 0; }
    int ok = (fwrite(t, sizeof(Train), 1, fp) == 1);
    fclose(fp);
    return ok;
}

static int append_train(const Train *t){
    FILE *fp = fopen(TRAINS_FILE, "ab");
    if(!fp) return 0;
    int ok = (fwrite(t, sizeof(Train), 1, fp) == 1);
    fclose(fp);
    return ok;
}

static int append_ticket(const Passenger *p){
    FILE *fp = fopen(TICKETS_FILE, "ab");
    if(!fp) return 0;
    int ok = (fwrite(p, sizeof(Passenger), 1, fp) == 1);
    fclose(fp);
    return ok;
}

static int delete_ticket_by_id(int ticket_id, Passenger *out_deleted){
    FILE *in = fopen(TICKETS_FILE, "rb");
    if(!in) return 0;
    FILE *tmp = fopen("tickets.tmp", "wb");
    if(!tmp){ fclose(in); return 0; }

    Passenger p; int found = 0;
    while (fread(&p, sizeof(Passenger), 1, in) == 1){
        if (!found && p.ticket_id == ticket_id){
            if (out_deleted) *out_deleted = p;
            found = 1; // skip writing this record
        } else {
            fwrite(&p, sizeof(Passenger), 1, tmp);
        }
    }
    fclose(in); fclose(tmp);

    if (found){
        remove(TICKETS_FILE);
        rename("tickets.tmp", TICKETS_FILE);
        return 1;
    } else {
        remove("tickets.tmp");
        return 0;
    }
}

static int next_ticket_id(){
    FILE *fp = fopen(TICKETS_FILE, "rb");
    int max_id = 1000; // start range
    if (!fp) return max_id + 1;
    Passenger p;
    while (fread(&p, sizeof(Passenger), 1, fp) == 1){
        if (p.ticket_id > max_id) max_id = p.ticket_id;
    }
    fclose(fp);
    return max_id + 1;
}

static int next_seat_no_for_train(int train_no){
    // Assign lowest available seat number based on existing tickets
    int seats_taken[1000] = {0}; // supports up to 1000 seats; adjust if needed

    FILE *fp = fopen(TICKETS_FILE, "rb");
    if (fp){
        Passenger p;
        while (fread(&p, sizeof(Passenger), 1, fp) == 1){
            if (p.train_no == train_no){
                if (p.seat_no >= 1 && p.seat_no <= 1000)
                    seats_taken[p.seat_no - 1] = 1;
            }
        }
        fclose(fp);
    }

    // Find first free seat number
    for (int i = 0; i < 1000; ++i){
        if (!seats_taken[i]) return i + 1;
    }
    return -1; // none available in range
}

// ===================== Seeding =====================
static void seed_trains_if_empty(){
    if (train_record_count() > 0) return;

    printf("No trains found. Seeding sample trains...\n");
    Train samples[] = {
        {101, "Express Alpha", "Delhi", "Mumbai", 5, 5, 1499.0f},
        {202, "Coastal Rider", "Chennai", "Kochi", 4, 4, 899.0f},
        {303, "Mountain Queen", "Dehradun", "Shimla", 6, 6, 799.0f}
    };
    size_t n = sizeof(samples)/sizeof(samples[0]);
    for (size_t i = 0; i < n; ++i) append_train(&samples[i]);
}

// ===================== Features =====================
static void add_train(){
    Train t;
    t.train_no = read_int("Enter train number: ");

    Train existing; long idx;
    if (find_train(t.train_no, &existing, &idx)){
        printf("Train %d already exists.\n", t.train_no);
        return;
    }

    read_line("Enter train name: ", t.name, sizeof(t.name));
    read_line("Enter source: ", t.source, sizeof(t.source));
    read_line("Enter destination: ", t.destination, sizeof(t.destination));
    t.seats_total = read_int("Enter total seats: ");
    t.seats_available = t.seats_total;
    t.fare = read_float("Enter fare: ");

    if (append_train(&t)) printf("Train added successfully.\n");
    else printf("Failed to add train.\n");
}

static void view_trains(){
    FILE *fp = fopen(TRAINS_FILE, "rb");
    if(!fp){ printf("No trains found.\n"); return; }
    printf("\n%-8s %-20s %-12s %-12s %-6s %-6s %-8s\n", "No", "Name", "Source", "Dest", "Total", "Avail", "Fare");
    printf("-------------------------------------------------------------------------------\n");
    Train t; int any = 0;
    while (fread(&t, sizeof(Train), 1, fp) == 1){
        any = 1;
        printf("%-8d %-20s %-12s %-12s %-6d %-6d %-8.2f\n", t.train_no, t.name, t.source, t.destination, t.seats_total, t.seats_available, t.fare);
    }
    if(!any) printf("No trains to display.\n");
    fclose(fp);
}

static void book_ticket(){
    int train_no = read_int("Enter train number to book: ");
    Train t; long idx;
    if (!find_train(train_no, &t, &idx)){
        printf("Train not found.\n");
        return;
    }
    if (t.seats_available <= 0){
        printf("No seats available on this train.\n");
        return;
    }

    Passenger p;
    p.ticket_id = next_ticket_id();
    p.train_no = train_no;
    p.seat_no = next_seat_no_for_train(train_no);
    if (p.seat_no < 1 || p.seat_no > t.seats_total){
        // Fallback: allocate based on remaining seats
        p.seat_no = t.seats_total - t.seats_available + 1;
        if (p.seat_no < 1) p.seat_no = 1;
    }
    read_line("Enter passenger name: ", p.name, sizeof(p.name));
    p.age = read_int("Enter age: ");

    if (!append_ticket(&p)){
        printf("Failed to write ticket.\n");
        return;
    }

    // Update seats_available
    t.seats_available -= 1;
    if (!write_train_at(idx, &t)){
        printf("Warning: booked, but failed to update seat availability.\n");
    }

    printf("\n*** Ticket Booked Successfully ***\n");
    printf("Ticket ID : %d\n", p.ticket_id);
    printf("Train No  : %d (%s %s->%s)\n", t.train_no, t.name, t.source, t.destination);
    printf("Seat No   : %d\n", p.seat_no);
    printf("Fare      : %.2f\n", t.fare);
}

static void cancel_ticket(){
    int ticket_id = read_int("Enter ticket ID to cancel: ");
    Passenger del;
    if (!delete_ticket_by_id(ticket_id, &del)){
        printf("Ticket not found.\n");
        return;
    }

    // Increase seat availability on the train
    Train t; long idx;
    if (find_train(del.train_no, &t, &idx)){
        if (t.seats_available < t.seats_total) t.seats_available += 1;
        write_train_at(idx, &t);
    }

    printf("Ticket %d cancelled successfully.\n", ticket_id);
}

static void view_tickets(){
    FILE *fp = fopen(TICKETS_FILE, "rb");
    if(!fp){ printf("No tickets booked yet.\n"); return; }
    printf("\n%-8s %-20s %-4s %-8s %-6s\n", "TID", "Name", "Age", "TrainNo", "Seat");
    printf("-------------------------------------------------------------\n");
    Passenger p; int any = 0;
    while (fread(&p, sizeof(Passenger), 1, fp) == 1){
        any = 1;
        printf("%-8d %-20s %-4d %-8d %-6d\n", p.ticket_id, p.name, p.age, p.train_no, p.seat_no);
    }
    if(!any) printf("No tickets to display.\n");
    fclose(fp);
}

static void search_train_menu(){
    int train_no = read_int("Enter train number to search: ");
    Train t; long idx;
    if (find_train(train_no, &t, &idx)){
        printf("\nFound Train:\n");
        printf("No: %d\nName: %s\nRoute: %s -> %s\nSeats: %d total, %d available\nFare: %.2f\n",
               t.train_no, t.name, t.source, t.destination, t.seats_total, t.seats_available, t.fare);
    } else {
        printf("Train not found.\n");
    }
}

// ===================== Menu =====================
static void print_menu(){
    printf("\n====== Railway Reservation System ======\n");
    printf("1. Add Train\n");
    printf("2. View Trains\n");
    printf("3. Book Ticket\n");
    printf("4. Cancel Ticket\n");
    printf("5. View Booked Tickets\n");
    printf("6. Search Train by Number\n");
    printf("0. Exit\n");
}

int main(void){
    // Ensure files exist
    FILE *f;
    if ((f = fopen(TRAINS_FILE, "ab"))) fclose(f);
    if ((f = fopen(TICKETS_FILE, "ab"))) fclose(f);

    seed_trains_if_empty();

    while(1){
        print_menu();
        int choice = read_int("Enter your choice: ");
        switch(choice){
            case 1: add_train(); pause_enter(); break;
            case 2: view_trains(); pause_enter(); break;
            case 3: book_ticket(); pause_enter(); break;
            case 4: cancel_ticket(); pause_enter(); break;
            case 5: view_tickets(); pause_enter(); break;
            case 6: search_train_menu(); pause_enter(); break;
            case 0: printf("Goodbye!\n"); return 0;
            default: printf("Invalid choice. Try again.\n"); pause_enter(); break;
        }
    }
}
