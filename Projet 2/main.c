#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define POWER_FILE "Data/power.csv"
#define EQUIPMENT_FILE "house_config.txt"
#define MONTHLY_DATA "Data/Monthlydata_43.455_5.471_SA_2016_2016.csv"
#define MAX_WORD_LENGHT	28		/* Maximum word length */
#define HASH_SIZE 61	/* Prime number */
#define BASE 128
#define SAUVEGARDE "save.txt"


typedef struct _element{
  char word[MAX_WORD_LENGHT];
  unsigned int power;
  struct _element *next;
}Element;

typedef struct _hash_table{
  unsigned int size;
  unsigned int nb_occupied_entries;
  unsigned int nb_elements;
  Element** Elements;
}HashTable;

typedef struct _equipment{
  char word[MAX_WORD_LENGHT];
  unsigned int number;
  unsigned int hours_per_week;
  struct _equipment *next;
}Equipment;

typedef struct{
  unsigned int house_surface;
  unsigned int exploitable_surface;
  Equipment *head;
}HouseConfig;


void menu(HashTable* hash_power, HouseConfig* house_config);
void menu_house_config(HouseConfig* house_config);

void initialize_hash_table(HashTable* hash_tab);
void load_hash_table(HashTable* hash_tab);
void insert_element_to_hash_table(HashTable* hash_tab, Element* element);
unsigned long get_hash_value(char *string);
void print_hash_table_characteristics(HashTable* hash_tab);

Equipment* init_equipment(void);
void get_word(char* word);
void add_equipment(HouseConfig* house_config, Equipment* equipment);
void supress_element(char* word, HouseConfig *house_config);
void supress_equipment(HouseConfig *house_config);
void load_house_config(HouseConfig* house_config);
void disp_equipment(Equipment *equipment);
void disp_house_config(HouseConfig* house_config);
void save_house_config(HouseConfig* HouseConfig);

void compute(HashTable* hash_power, HouseConfig* house_config);
unsigned int calc_mean_consumption(HashTable* hash_power, HouseConfig* house_config);
float calc_kWc(int N);
unsigned int surface_necessary(unsigned int consumption_house);
void save_result(unsigned int consumption, float price_consumption, int surface, unsigned int exploitable_surface, int N, unsigned int price, float production_solar_panel, float money_save, int time_refund);
void change_data(int price, float price2);

/****************************** Main functions ********************************/

int main(){
  // Initializes structures and launches main menu
	HashTable hash_power;
	initialize_hash_table(&hash_power);
  load_hash_table(&hash_power);
  print_hash_table_characteristics(&hash_power);
  HouseConfig house_config;
	house_config.head=NULL;
  menu(&hash_power, &house_config);
	return 0;
}


void menu(HashTable* hash_power, HouseConfig* house_config){
  // Main menu
  int choice;
  printf("\n***************************** Menu *****************************\n");
  printf("1/configurateur de logement\n2/simulation\n3/changement des prix\n");
  printf("****************************************************************\nchoice: ");
  scanf("%d", &choice);
  printf("\n\n");
  int price;
  float price2;
  switch (choice){
    case 1 :
      menu_house_config(house_config);
      menu(hash_power, house_config);
      break;
    case 2 :
      compute(hash_power, house_config);
      menu(hash_power, house_config);
      break;
    case 3 :
      printf("Donner le prix d'un panneau solaire: ");
      scanf("%d",&price);
      printf("Donner le prix du kWh");
      scanf("%f",&price2);
      change_data(price, price2);
      break;
  }
}


void menu_house_config(HouseConfig* house_config){
  // House configuration menu
  int choice;
  printf("\n********************** Configurateur de logement **********************\n");
  printf("1/charger depuis le fichier \n2/ajouter un équipement\n3/supprimer un équipement\n4/afficher les équipements\n5/sauvegarder la configuration\n6/menu principal\n");
  printf("****************************************************************\nchoix: ");
  scanf("%d", &choice);
  printf("\n\n");
  switch (choice){
    case 1 :
      load_house_config(house_config);
      menu_house_config(house_config);
      break;
    case 2 :
      add_equipment(house_config, init_equipment());
      printf(">>> L'équipement a été ajouté avec succès");
      menu_house_config(house_config);
      break;
    case 3 :
      supress_equipment(house_config);
      menu_house_config(house_config);
      break;
    case 4 :
      disp_house_config(house_config);
      menu_house_config(house_config);
      break;
    case 5 :
      save_house_config(house_config);
      menu_house_config(house_config);
      break;
    case 6 :
      break;
  }
}

/******************************************************************************/

/******************************** Hash table **********************************/
// general functions used to interact with the hash table
unsigned long get_hash_value(char *string){
	unsigned long	hash_value = 0;
	int	i = 0;
	while ((*string) != '\0')
	{
		hash_value += hash_value % HASH_SIZE + ((*string) * (int) pow (BASE, i) )% HASH_SIZE;
		i++;
		string++;
	}
	return hash_value % HASH_SIZE;
}


void insert_element_to_hash_table(HashTable* hash_tab, Element* element){
	hash_tab->nb_elements++;
	unsigned long	i		= get_hash_value(element->word);
	if (hash_tab->Elements[i] == NULL)
		hash_tab->nb_occupied_entries++;
	element->next = hash_tab->Elements[i];
	hash_tab->Elements[i]	= element;
}


void initialize_hash_table(HashTable *hash_tab){
	hash_tab->size	= HASH_SIZE;
	hash_tab->nb_occupied_entries = 0;
	hash_tab->nb_elements = 0;
	hash_tab->Elements = (Element **) malloc(hash_tab->size * sizeof(Element*));
	for (unsigned int i = 0; i < hash_tab->size; i++)
		hash_tab->Elements[i] = NULL;
}


void load_hash_table(HashTable *hash_tab){
  FILE* file;
  file = fopen(POWER_FILE, "r");
  if (file!=NULL){
    unsigned int power;
    char word[MAX_WORD_LENGHT];
    while (fscanf(file, "%s %u", word, &power) > 0){
      Element* element	= (Element *) malloc(sizeof(Element));
      strcpy(element->word, word);
      element->power = power;
      insert_element_to_hash_table(hash_tab, element);
    }
  }
  else
    printf("\nfichier power.txt introuvable\n");
  fclose(file);
}


void print_hash_table_characteristics(HashTable* hash_tab){
	float loadFactor =  1.0*hash_tab->nb_occupied_entries/hash_tab->size;
	printf("************** Caractéristiques de la table de hachage **********************\n");
	printf("Nombre d'entrées: %u \n", hash_tab->size);
	printf("Facteur de remplissage: %.2f \n", loadFactor);
	printf("Nombre total d'éléments: %u \n", hash_tab->nb_elements);
	printf("****************************************************************\n");
}

/******************************************************************************/

/**************************** House configuration *****************************/

Equipment* init_equipment(){
  // allows the user to enter the specifications of a new equipment he wants to
  // add to his house
  char word[MAX_WORD_LENGHT];
  printf("Entrer le nom de l'équipement: ");
  scanf("%s", word);
  unsigned int number;
  printf("\nNombre d'unités dans le logement: ");
  scanf("%u", &number);
  unsigned int hours_per_week;
  printf("\nNombre d'heures d'utilisation par semaine: ");
  scanf("%u", &hours_per_week);
  printf("\n");
  Equipment *equipment = (Equipment *) malloc(sizeof(Equipment));
  equipment->number = number;
  strcpy(equipment->word, word);
  equipment->hours_per_week = hours_per_week;
  return equipment;
}


void add_equipment(HouseConfig* house_config, Equipment* equipment){
  // adds an equipment to the linked list of equipments in house_config
  equipment->next = house_config->head;
  house_config->head = equipment;
}


void get_word(char* word){
  // gets the name of the equipment to remove from the configuration
  printf("Entrer le nom de l'équipement à supprimer: ");
  scanf("%s", word);
}


void supress_element(char* word, HouseConfig *house_config){
  // removes the equipment named [word] of the list of equipments in house_config
  Equipment *current = house_config->head;
  Equipment *prec = NULL;
  while (current != NULL && strcmp(current->word, word) != 0){
    prec = current;
    current = current->next;
  }
  if (current == NULL)
    printf(">>> nom d'équipement invalide\n");
  else{
    if (prec == NULL)
      house_config->head = current->next;
    else
      prec->next = current->next;
    printf(">>> l'équipement %s a été supprimé avec succés\n", word);
  }
}

void supress_equipment(HouseConfig *house_config){
  // removes an equipment of the linked list of equipments in house_config
  char word[MAX_WORD_LENGHT];
  get_word(word);
  supress_element(word, house_config);
}


void load_house_config(HouseConfig* house_config){
  // load the house's configuration stored in the file 'house_config.txt'
  FILE* file;
  file = fopen(EQUIPMENT_FILE, "r");
  if (file != NULL){
    unsigned int number;
    char word[MAX_WORD_LENGHT];
    unsigned int hours_per_week;
    fscanf(file,"%s %u",word, &house_config->house_surface);
    fscanf(file,"%s %u",word, &house_config->exploitable_surface);
    while (fscanf(file, "%s %u %u", word, &number, &hours_per_week) == 3){
        Equipment *equipment = (Equipment *) malloc(sizeof(Equipment));
        equipment->number = number;
        strcpy(equipment->word, word);
        equipment->hours_per_week = hours_per_week;
        add_equipment(house_config, equipment);
    }
    printf(">>> fichier de configuration chargé avec succés\n");
    fclose(file);
  }
  else
    printf(">>> fichier house_config.txt introuvable\n");
}

void save_house_config(HouseConfig* house_config){
  // saves the house_config in the file 'house_config.txt'
  FILE* file;
  file = fopen(EQUIPMENT_FILE, "w");
  fprintf(file, "house_surface %u\n", house_config->house_surface);
  fprintf(file, "exploitable_surface %u\n", house_config->exploitable_surface);
  Equipment *current;
  current = house_config->head;
  while (current != NULL){
    fprintf(file, "%s %u %u\n", current->word, current->number, current->hours_per_week);
    current = current->next;
  }
  fclose(file);
  printf(">>> Configuration sauvegardée dans le fichier \"house_config.txt\"\n");
}

void disp_equipment(Equipment *equipment){
  // displays an equipment
  printf("équipement: %s, nombre d'unités: %u, heures d'utilisation par semaine: %u\n", equipment->word, equipment->number, equipment->hours_per_week);
}

void disp_house_config(HouseConfig* house_config){
  // prints house_config
  Equipment *current = house_config->head;
  printf("****************** Configuration du logement **************************\n");
  printf("Surface du logement (m^2): %u\n", house_config->house_surface);
  printf("Surface exploitable (m^2): %u\n", house_config->exploitable_surface);
  printf(">>> Équipements:\n");
  if (current == NULL)
    printf("Pas d'équipements à afficher\n");
  else{
    while ((current != NULL)){
      disp_equipment(current);
      current = current->next;
    }
  }
  printf("****************************************************************\n");
}

/******************************************************************************/

/******************************** Computation *********************************/

void compute(HashTable* hash_power, HouseConfig* house_config){
  // main simulation function
  //calculation of the mean annual consumption of the house in kWh
  unsigned int mean_consumption = calc_mean_consumption(hash_power, house_config);
  printf("Consommation moyenne: %u\n", mean_consumption);
  //loading the price of a solar panel and of the kWh
  FILE* file;
  file = fopen("Data/data.txt", "r");
  unsigned int solar_panel_cost;
  float price_kwh;
  if (file != NULL){
    char type[MAX_WORD_LENGHT];
    fscanf(file, "%s %u", type, &solar_panel_cost);
    fscanf(file,"%s %f", type, &price_kwh);
  }
  fclose(file);
  //returns the price of the consumption of the house
  float price_consumption=(mean_consumption)*price_kwh;
  printf("La consommation électrique de la maison revient à %f euros par an.\n", price_consumption);
  //returns the surface necessary for the solar panel to produce the power needed by the house
  unsigned int surface=surface_necessary(mean_consumption);
  printf("Il faut utiliser une surface de %u m² pour compenser cette consommation.\n",surface);
  printf("Combien de m² voulez vous utiliser pour l'installation de panneaux solaires ? \n");
  scanf("%u",&(house_config->exploitable_surface));
  //returns the number of solar panel needed
  int N=(house_config->exploitable_surface)/1.7; /*dimension of a solar panel : 1.7 m²*/
  printf("On peut installer au maximum %d panneaux solaires sur cette surface.\n",N);
  //returns the price of the installation
  unsigned int price=N*solar_panel_cost;
  printf("Cette installation coûterait %u euros.\n",price);
  //returns the production of the solar panel in kWh par an
  float production_solar_panel=calc_kWc(N);
  printf("Si les panneaux solaires sont produisent au maximum de leur capacité, ils génèreront %f kWh par an.\n",production_solar_panel);
  float money_save=production_solar_panel*price_kwh;
  //returns the money save by the installation
  printf("Cela se traduira par une économie de %f euros par an\n",money_save);
  //returns the time needed to refund the installation
  int time_refund=(price/money_save)+1;
  printf("L'installation sera ainsi remboursée au cours de la %d année.\n",time_refund);
  //saves the result in the file save.txt
  save_result(mean_consumption, price_consumption, surface, house_config->exploitable_surface, N, price, production_solar_panel, money_save, time_refund);
}


unsigned int calc_mean_consumption(HashTable* hash_power, HouseConfig* house_config){
  // returns the mean annual consumption of the house in kWh.
  Equipment* current = house_config->head;
  unsigned int result = 0;
  unsigned int intermediate;
  int power;
  while (current != NULL){
    power = hash_power->Elements[get_hash_value(current->word)]->power;
    intermediate = power*(current->number)*(current->hours_per_week)*52;
    if (strcmp(current->word, "chauffage") == 0)
      result += (intermediate)*(house_config->house_surface);
    else
      result += intermediate;
    current = current->next;
  }
  return (result/1000);
}


unsigned int surface_necessary(unsigned int consumption_house){
  // returns the surface necessary to produce the consumption of the house with
  // solar panel
  FILE* file;
  file=fopen(MONTHLY_DATA,"r");
  if(file!=NULL){
    char type[MAX_WORD_LENGHT];
    char type2[MAX_WORD_LENGHT];
    char type3[MAX_WORD_LENGHT];
    char type4[MAX_WORD_LENGHT];
    float passer;
    fscanf(file,"%s %s %s %f",type, type2, type3, &passer);
    fscanf(file,"%s %s %s %f",type, type2, type3, &passer);
    fscanf(file,"%s %s %s",type, type2, type3);
    fscanf(file,"%s %s %s  %s %f",type, type2, type3,type4, &passer);
    fscanf(file,"%s %s %s ",type, type2, type3);
    float kwc=0.0;
    float production_month;
    int year;
    char month[MAX_WORD_LENGHT];
    for(int i=0;i<12;i++){
    	fscanf(file,"%d %s %f",&year, month , &production_month);
    	kwc=kwc+production_month;
    }
    fclose(file);
    unsigned int surface_necessary=consumption_house/kwc+2;
    return surface_necessary;
    }
  else{
  	fclose(file);
  	return 0;
  }
}


float calc_kWc(int N)
{
  //returns the power produced by N solar panel during a year
  FILE* file;
  file=fopen(MONTHLY_DATA,"r");
  if(file!=NULL){
  	char type[MAX_WORD_LENGHT];
  	char type2[MAX_WORD_LENGHT];
  	char type3[MAX_WORD_LENGHT];
  	char type4[MAX_WORD_LENGHT];
  	float passer;
  	fscanf(file,"%s %s %s %f",type, type2, type3, &passer);
  	fscanf(file,"%s %s %s %f",type, type2, type3, &passer);
  	fscanf(file,"%s %s %s",type, type2, type3);
  	fscanf(file,"%s %s %s  %s %f",type, type2, type3,type4, &passer);
  	fscanf(file,"%s %s %s ",type, type2, type3);
  	float kwc=0.0;
  	float production_month;
  	int year;
  	char month[MAX_WORD_LENGHT];
  	for(int i=0;i<12;i++){
  		fscanf(file,"%d %s %f",&year, month , &production_month);
  		kwc=kwc+production_month;
  		}
  	fclose(file);
  	return kwc*N*1.7;
  }
  else{
  	fclose(file);
  	return 0.0;
  }
}


void change_data(int price, float price2){
  // updates the price of the solar panel and of the kWh
	FILE* file;
  	file = fopen("Data/data.txt", "w");
  	fprintf(file,"solar_panel_cost_per_kWc %d  \n",price);
  	fprintf(file,"price_kWh %f  ", price2);
  	printf(">>> Les prix ont bien été changés");
 }


void save_result(unsigned int consumption, float price_consumption, int surface, unsigned int exploitable_surface, int N, unsigned int price, float production_solar_panel, float money_save, int time_refund){
  // saves the result of the simulation
  FILE* file;
 	file=fopen(SAUVEGARDE,"w");
 	fprintf(file,"La consomation de la maison est %u kWh par an.\n",consumption);
 	fprintf(file,"Cela coute %f par an.\n",price_consumption);
 	fprintf(file,"Pour compenser cette consommation on a besoin de %u m² pour installer des panneaux solaires.\n",surface);
 	fprintf(file,"Vous avez decider d'utiliser %u metres carrés pour votre installation.",exploitable_surface);
 	fprintf(file,"Vous pouvez donc installer %d panneaux solaires pour %u euros.\n",N, price);
 	fprintf(file,"Ces panneaux solaires vont produire %f kWh par an.\n",production_solar_panel);
 	fprintf(file,"Ce qui se traduira par une économie de %f euros par an.\n",money_save);
 	fprintf(file,"Il vous faudra donc %d années pour rentabiliser l'installation.",time_refund);
 	fclose(file);
 }
/******************************************************************************/
