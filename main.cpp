#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <netinet/ether.h>
#include <iostream>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

#define BUFSIZE 1000

using namespace std;

int main(int argc, char *argv[]) {
    std::string text = ""; // Přijímaný text
    sockaddr_in sockName; // "Jméno" portu
    sockaddr_in clientInfo; // Klient, který se připojil 
    int mainSocket; // Soket
    int port; // Číslo portu
    char buf[BUFSIZE]; // Přijímací buffer
    int size; // Počet přijatých a odeslaných bytů
    socklen_t addrlen; // Velikost adresy vzdáleného počítače
    int count = 0; // Počet připojení

    if (argc != 2) {
        cerr << "Syntaxe:\n\t" << argv[0]
                << " " << "port" << endl;
        return -1;
    }
    port = atoi(argv[1]);
    // Vytvoříme soket - viz minulý díl
    if ((mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        cerr << "Nelze vytvořit soket" << endl;
        return -1;
    }
    // Zaplníme strukturu sockaddr_in
    // 1) Rodina protokolů
    sockName.sin_family = AF_INET;
    // 2) Číslo portu, na kterém čekáme
    sockName.sin_port = htons(port);
    // 3) Nastavení IP adresy lokální síťové karty, přes kterou je možno se
    //    připojit. Nastavíme možnost připojit se odkudkoliv. 
    sockName.sin_addr.s_addr = INADDR_ANY;
    // přiřadíme soketu jméno
    if (bind(mainSocket, (sockaddr *) & sockName, sizeof (sockName)) == -1) {
        cerr << "Problém s pojmenováním soketu." << endl;
        return -1;
    }
    // Vytvoříme frontu požadavků na spojení. 
    // Vytvoříme frontu maximální velikosti 10 požadavků.
    if (listen(mainSocket, 10) == -1) {
        cerr << "Problém s vytvořením fronty" << endl;
        return -1;
    }
    // Poznačím si velikost struktury clientInfo.
    // Předám to funkci accept. 
    addrlen = sizeof (clientInfo);
    // Vyberu z fronty požadavek na spojení.
    // "client" je nový soket spojující klienta se serverem.

    do {
        int client = accept(mainSocket, (sockaddr*) & clientInfo, &addrlen);
        int can_end = 0;
        if (client == -1) {
            cerr << "Problém s přijetím spojeni" << endl;
            return -1;
        }
        // Zjistím IP klienta.
        cout << "Někdo se připojil z adresy: "
                << inet_ntoa((in_addr) clientInfo.sin_addr) << endl;
        // Přijmu data. Ke komunikaci s klientem používám soket "client"
        text += inet_ntoa((in_addr) clientInfo.sin_addr);
        text += ": ";
        std::string act_text = "";
        // Přijmeme maximálně 6 bytový pozdrav. 
        while (can_end == 0) {
            if ((size = recv(client, buf, BUFSIZE - 1, 0)) == -1) {
                cerr << "Problém s přijetím dat." << endl;
                return -1;
            }
            if (size == 0) can_end = 1;
            if (can_end == 1) break;
//            cout << "Přijato: " << size << endl;
//            cout << "Data: " << buf << endl;
            //            if (totalSize == 6) can_end = 11;
            act_text += buf;
            if (act_text.length() < BUFSIZE) can_end = 1;
            unsigned endPoint = act_text.rfind('\n');
            if (endPoint != 0) can_end = 1;
//            cout << endl << "can end: " << can_end << "text: " << text << endl;
            if (can_end == 1) break;
            //            text += buf;
        }
        text += act_text;
        cout << text;
        text += "\n";
        // Odešlu pozdrav
        cout << endl << "Chci odeslat" << endl << text << endl;

        if ((size = send(client, text.c_str(), text.length() + 1, 0)) == -1) {
            cerr << "Problém s odesláním dat" << endl;
            return -1;
        }
//        cout << "Odesláno: " << size << endl;
        close(client);
        // Uzavřu spojení s klientem
    } while (true);
    close(mainSocket);
    return 0;
}