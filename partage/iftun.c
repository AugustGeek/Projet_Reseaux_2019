#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h> 
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include "iftun.h"
#include "extremite.h"  // IMPORT DE LA BIBLIOTHEQUE extremite

#define MAXBUFLEN 1500


int tun_alloc(char *dev)
{
    struct ifreq ifr;
    int fd, err;

    if( (fd = open("/dev/net/tun", O_RDWR)) < 0 ){
      perror("alloc tun");
      exit(-1);
    }

    memset(&ifr, 0, sizeof(ifr));

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers) 
     *        IFF_TAP   - TAP device  
     *
     *        IFF_NO_PI - Do not provide packet information  
     */ 
    ifr.ifr_flags = IFF_TUN; 
    if( *dev )
      strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
      close(fd);
      return err;
    }
    strcpy(dev, ifr.ifr_name);
    return fd;
}

int copierDescripteur(int src, int dst){
  int nbytesSRC, nbytesDST ;
  char monBuffer[MAXBUFLEN] ;

  if(dst != 1){
    system("sudo touch /dev/net/tunfd_dst");  // CREATION DU FICHIER LIE AU DESCRIPTEUR DE FICHIER dst tunfd_dst
    system("sudo chmod +x /dev/net/tunfd_dst");  // EDITION DES PERMISSION D'ACCES
    if( (dst = open("/dev/net/tunfd_dst", O_RDWR)) == -1 ){
        perror("Fichier copie inaccessible");
        exit(-1);
      }
  }

  while(1) {
       nbytesSRC = read( src, monBuffer, sizeof(monBuffer) );
       printf("Read %d bytes from tun0 \n", nbytesSRC);
       nbytesDST = write(dst, monBuffer, sizeof(monBuffer));
       printf("Write %d bytes to tunfd_dst \n", nbytesDST);
     }

  return 0;
}      

int main (int argc, char **argv){


  if(argc==2){  // COTE SERVEUR

    ext_out( argv[1] );

  }else{ // COTE CLIENT

    int tunfd, tunfd_dst; // Si tunfd_dst=1, LES DONNEES SERONT AFFICHEES SUR LA SORTIE STANDARD

    printf("Création de %s\n",argv[1]);

    tunfd = tun_alloc(argv[1]); // argv[1]=NOM DU TUNNEL. tunfd EST LES DESCRIPTEUR DE FICHIER DU TUNNEL
    printf("Tunnel %s créée \n", argv[1]);

    printf("Faire la configuration de %s...\n",argv[1]);
    printf("Appuyez sur une touche pour continuer\n");
    getchar();

    system("./configure-tun.sh"); // EXECUTION DU SCRIPT BASH
    printf("Interface %s Configurée:\n",argv[1]);

    printf("Appuyez sur une touche pour terminer\n");
    getchar();

    //COPIE DES DONNEES DU DESCRIPTEUR DE FICHIER tunfd de tun0
    
    //copierDescripteur(tunfd, tunfd_dst); // APPEL DE LA METHODE CHARGEE DE RECOPIER LES DONNEES DE tunfd DANS UN AUTRE FICHIER OU DANS LA SORTIE STANDARD
    
    ext_in(tunfd, argv[2], argv[3]);  // argv[2] EST L'ADRESSE IP DU SERVEUR POUR CREER LA SOCKET. argv[3] EST LE PORT SUR LEQUEL ECOUTE LE SERVEUR
  }

    return 0;
}
