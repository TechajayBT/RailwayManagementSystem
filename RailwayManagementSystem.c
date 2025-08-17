#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Train {
    int train_no;
    char name[50];
    char source[30];
    char destination[30];
    int seats;
    float fare;
};

struct Passenger {
    int ticket_id;
    char name[50];
    int age;
    int train_no;
    int seat_no;
};

// File names
#define TRAIN_FILE "trains.dat"
#define TICKET_FILE "tickets.dat"

// Function prototypes
void addTrain();
void viewTrains();
void deleteTrain();
void bookTicket();
void cancelTicket();
void viewBookedTickets();

void addTrain() {
    FILE *fp = fopen(TRAIN_FILE, "ab+");
    if (!fp) {
        printf("Error opening file!\n");
        return;
    }
    struct Train t, temp;
    int duplicate = 0;
    printf("Enter Train No: "); scanf("%d", &t.train_no);

    // Check for duplicate train number
    rewind(fp);
    while (fread(&temp, sizeof(temp), 1, fp)) {
        if (temp.train_no == t.train_no) {
            duplicate = 1;
            break;
        }
    }
    if (duplicate) {
        printf("Train number already exists!\n");
        fclose(fp);
        return;
    }

    printf("Enter Train Name: "); scanf(" %[^\n]", t.name);
    printf("Enter Source: "); scanf(" %[^\n]", t.source);
    printf("Enter Destination: "); scanf(" %[^\n]", t.destination);
    printf("Enter Total Seats: "); scanf("%d", &t.seats);
    printf("Enter Fare: "); scanf("%f", &t.fare);

    fwrite(&t, sizeof(t), 1, fp);
    fclose(fp);
    printf("Train added successfully!\n");
}

void viewTrains() {
    FILE *fp = fopen(TRAIN_FILE, "rb");
    if (!fp) {
        printf("No trains found!\n");
        return;
    }
    struct Train t;
    printf("\n%-10s %-20s %-15s %-15s %-10s %-10s\n", "TrainNo", "Name", "Source", "Destination", "Seats", "Fare");
    while (fread(&t, sizeof(t), 1, fp)) {
        printf("%-10d %-20s %-15s %-15s %-10d %-10.2f\n", t.train_no, t.name, t.source, t.destination, t.seats, t.fare);
    }
    fclose(fp);
}

void deleteTrain() {
    int train_no, found = 0;
    printf("Enter Train No to delete: ");
    scanf("%d", &train_no);

    FILE *fp = fopen(TRAIN_FILE, "rb");
    FILE *temp = fopen("temp.dat", "wb");

    if (!fp || !temp) {
        printf("Error opening file!\n");
        return;
    }

    struct Train t;
    while (fread(&t, sizeof(t), 1, fp)) {
        if (t.train_no == train_no) {
            found = 1;
            char confirm;
            printf("Are you sure you want to delete Train No %d? (y/n): ", train_no);
            scanf(" %c", &confirm);
            if (confirm == 'y' || confirm == 'Y') {
                continue; // Skip this train (delete)
            } else {
                fwrite(&t, sizeof(t), 1, temp); // Don't delete
                found = 0;
            }
        } else {
            fwrite(&t, sizeof(t), 1, temp);
        }
    }

    fclose(fp);
    fclose(temp);

    remove(TRAIN_FILE);
    rename("temp.dat", TRAIN_FILE);

    if (found)
        printf("Train deleted successfully!\n");
    else
        printf("Train not found or deletion cancelled!\n");
}

void bookTicket() {
    FILE *fp = fopen(TRAIN_FILE, "rb+");
    FILE *tp = fopen(TICKET_FILE, "ab");
    if (!fp || !tp) {
        printf("Error opening file!\n");
        return;
    }

    int train_no;
    printf("Enter Train No: ");
    scanf("%d", &train_no);

    struct Train t;
    int found = 0;
    while (fread(&t, sizeof(t), 1, fp)) {
        if (t.train_no == train_no) {
            found = 1;
            if (t.seats > 0) {
                struct Passenger p;
                printf("Enter Passenger Name: "); scanf(" %[^\n]", p.name);
                printf("Enter Age: "); scanf("%d", &p.age);
                p.train_no = train_no;
                p.seat_no = t.seats;
                p.ticket_id = rand() % 10000 + 1;

                fwrite(&p, sizeof(p), 1, tp);

                // Update train seat count
                t.seats--;
                fseek(fp, -sizeof(t), SEEK_CUR);
                fwrite(&t, sizeof(t), 1, fp);
                printf("Ticket booked! Ticket ID: %d, Seat No: %d\n", p.ticket_id, p.seat_no);
            } else {
                printf("No seats available!\n");
            }
            break;
        }
    }
    if (!found) printf("Train not found!\n");

    fclose(fp);
    fclose(tp);
}

void cancelTicket() {
    int ticket_id, found = 0;
    printf("Enter Ticket ID to cancel: ");
    scanf("%d", &ticket_id);

    FILE *fp = fopen(TICKET_FILE, "rb");
    FILE *temp = fopen("temp.dat", "wb");
    FILE *tf = fopen(TRAIN_FILE, "rb+");

    if (!fp || !temp || !tf) {
        printf("Error opening file!\n");
        return;
    }

    struct Passenger p;
    while (fread(&p, sizeof(p), 1, fp)) {
        if (p.ticket_id == ticket_id) {
            found = 1;

            // increase train seats
            struct Train t;
            while (fread(&t, sizeof(t), 1, tf)) {
                if (t.train_no == p.train_no) {
                    t.seats++;
                    fseek(tf, -sizeof(t), SEEK_CUR);
                    fwrite(&t, sizeof(t), 1, tf);
                    break;
                }
            }
            continue; // Skip writing this ticket (cancel)
        }
        fwrite(&p, sizeof(p), 1, temp);
    }

    fclose(fp);
    fclose(temp);
    fclose(tf);

    remove(TICKET_FILE);
    rename("temp.dat", TICKET_FILE);

    if (found)
        printf("Ticket canceled successfully!\n");
    else
        printf("Ticket not found!\n");
}

void viewBookedTickets() {
    FILE *fp = fopen(TICKET_FILE, "rb");
    if (!fp) {
        printf("No tickets booked!\n");
        return;
    }
    struct Passenger p;
    printf("\n%-10s %-20s %-5s %-10s %-10s\n", "TicketID", "Name", "Age", "TrainNo", "SeatNo");
    while (fread(&p, sizeof(p), 1, fp)) {
        printf("%-10d %-20s %-5d %-10d %-10d\n", p.ticket_id, p.name, p.age, p.train_no, p.seat_no);
    }
    fclose(fp);
}

int main() {
    int choice;
    while (1) {
        printf("\n===== Railway Reservation System =====\n");
        printf("1. Add Train\n");
        printf("2. View Trains\n");
        printf("3. Delete Train\n");
        printf("4. Book Ticket\n");
        printf("5. Cancel Ticket\n");
        printf("6. View Booked Tickets\n");
        printf("7. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // clear buffer

        switch (choice) {
            case 1: addTrain(); break;
            case 2: viewTrains(); break;
            case 3: deleteTrain(); break;
            case 4: bookTicket(); break;
            case 5: cancelTicket(); break;
            case 6: viewBookedTickets(); break;
            case 7: exit(0);
            default: printf("Invalid choice! Try again.\n");
        }
    }
    return 0;
}