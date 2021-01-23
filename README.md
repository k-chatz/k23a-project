
  
<h1  align="center">ΑΝΑΠΤΥΞΗ ΛΟΓΙΣΜΙΚΟΥ ΓΙΑ ΠΛΗΡΟΦΟΡΙΑΚΑ ΣΥΣΤΗΜΑΤΑ</h1>  
  
    
  
<p  align="center">  
  
<br>  
  
<b>Entity resolution</b>  
  
<br>  
  
</p>  
  
    
  
<p  align="center">  
  
<a  target="_blank"  href="https://k23a-prj-auth.herokuapp.com"><strong>Documentation</strong></a>  
  
<br>  
  
</p>  
  
    
  
<p  align="center">  
  
<a  href="https://discord.gg/nSGmntAX">  
  
<img  alt="Discord"  src="https://img.shields.io/discord/765286458243153950?color=7389d8&label=Discord&logo=Discord&logoColor=fff">  
  
</a>  
  
</p>  
  
    
  
<hr>  
  
    
  
# Εθνικό και Καποδιστριακό Πανεπιστήμιο Αθηνών  
  
    
### Τμήμα Πληροφορικής και Τηλεπικοινωνιών  
  
    
### Κ23Α - ΑΝΑΠΤΥΞΗ ΛΟΓΙΣΜΙΚΟΥ ΓΙΑ ΠΛΗΡΟΦΟΡΙΑΚΑ ΣΥΣΤΗΜΑΤΑ  
  
    
### **Project - Entity resolution**  
  
    
**Μέλη**:  
  
    
  
- Θεόδωρος Χατζηιωαννίδης - 1115201600197  
  
- Βασίλειος Πουλόπουλος - 1115201600141  
  
- Κωνσταντίνος Χατζόπουλος - 1115201300202  
  
    
  
## Περιεχόμενα  
  
    
1. [PART 2](#part2)  
  
2. [Εισαγωγή](#intro)  
  
3. [Hash table](#hash_table)  
  
4. [List](#list)  
  
5. [JSON Parser](#json_parser)  
  
6. [Spec to specs](#spec_to_specs)  

7. [Μηχανική Μάθηση](#machine_learning)
  
8. [Ροή του προγράμματος](#flow)  
  
9. [Unit tests](#unit_tests)  

10. [Documentation](#documentation)
  
11. [Συμπεράσματα](#conclusions)  
 
  
    
    
  
<a  name="part2"></a>  
  
    
    
    
  
## Κλήση Προγραμμάτων  
  
    
Για να εκτελεστούν τα πρόγραμμα από τερματικό τρέχουμε το script 'build_and_run.sh'.  
  
Τα ορίσματα που δέχεται το εκτελέσιμο για το training είναι τα εξής:  
  
  
- -dir { dataset X path }  
  
- -csv { dataset W path }  
  
- -sw { stopwords file path }  
  
- -m { tfidf | bow }  
  
- -ex {resources path }  
  
Τα ορίσματα που δέχεται το εκτελέσιμο για το predicting είναι τα εξής:  
  
 - -dir {user dataset X path}  
   
 - -csv {user dataset W path}  
   
 - -vocabulary {vocabulary path}   
   
 - -model {model path}  
  
### Ενδεικτική εκτέλεση προγράμματος Training  
  
    
>./project \  
-dir Datasets/camera_specs/2013_camera_specs \  
-csv Datasets/sigmod_large_labelled_dataset.csv \  
-sw resources/unwanted-words.txt \  
-m bow \  
-ex resources  
  
  
  
  
### Ενδεικτική εκτέλεση προγράμματος Predicting  
  
    
>  ./user -dir resources/user_json_files \  
 -csv resources/user_dataset.csv \ -vocabulary resources/vocabulary.csv \ -model resources/model.csv  
### Έξοδος προγράμματος Training  
  
    
  **predictions** και **f1 score** για το validation set  
 
### Έξοδος προγράμματος Testing  
  
  **predicitons** για το dataset που δίνει ο χρήστης.  
  
**Σημείωση**: Για να περάσουμε το model και το vocabulary από το ένα πρόγραμμα στο άλλο, τα γράψαμε σε αρχεία.  

<a  name="intro"></a>  
## Εισαγωγή  
  
Στόχος της άσκησης είναι να δημιουργήσουμε κλίκες γράφων από αγγελίες με τα ίδια προϊόντα σε διάφορα site. Για την υλοποίηση χρειάστηκε να δημιουργήσουμε τις παρακάτω δομές δεδομένων:  

-  **hash table**:  
  
-  **list**  
 
Επίσης, υλοποιήσαμε έναν **json parser** ώστε να συλλέξουμε τα δεδομένα των αγγελιών από τα json αρχεία.    

Επιπλέον, για τον έλεγχο ορθότητας του κώδικα, χρησιμοποιήσαμε τη βιβλιοθήκη **acutest.h** όπου δέχεται μια λίστα από κατάλληλες συναρτήσεις (tests) που γράψαμε.  
  
<a  name="hash_table"></a>  
## Hash table  
  
    
![hash table](https://raw.githubusercontent.com/vasilisp100/k23a-project/master/resources/hash.png?token=AMOC6IZ2FVNF77KBO6EZCBK7Z7ZWC)  
  
    
  
Για την υλοποίηση του hash table, χρησιμοποιήσαμε generic **open addressing hash table** με **random probing**.  
Πιο συγκεκριμένα, δεσμεύουμε ένα κομμάτι μνήμης το οποίο χωρίζεται σε μκρότερα ίσα μέρη ώστε στα οποία αποθηκεύονται οι πληροφορίες του κάθε **spec**.  

Αφού χρησιμοποιούμε random probing το κάθε spec ανάλογα με το key του, αν δε χωράει στην πρώτη θέση που θα υπολογιστεί μέσω του hash function, θα ψάξει κάποια "τυχαία" θέση για να αποθηκευθεί. Το seed της random συνάρτησης είναι σταθερό, οπότε πάντα μπορούμε να υπολογίσουμε σε ποιο σημείο της μνήμης βρίσκονται τα εκάστοτε δεδομένα.  

Αν η μνήμη που έχουμε δεσμεύσει αρχίσει να γεμίζει θα κάνουμε **Rehashing**. Όταν θέλουμε να εισάγουμε νέα δεδομένα αλλά η πληρότητα του είναι 70%, δεσμεύουμε τη διπλάσια μνήμη από πριν, ξαναμοιράζουμε τα δεδομένα στην καινούργια, μεγαλύτερη μνήμη και εισάγουμε τα νέα δεδομένα.  
  
Η δομή που χρησιμοποιήσαμε για το hash table είναι η παρακάτω:  
```c  
  
typedef  struct htab_s {  
	ht_hash_func h;  
	ht_cmp_func cmp; 
    ht_key_cpy_func keycpy; 
	size_t key_sz; 
	size_t val_sz; 
	ulong buf_cap;  
	ulong buf_load; 
	char buf[]; 
} htab_t; 
```  
  
<a  name="list"></a>  
  
## List  
  
Χρησιμοποιήσαμε μία generic **list**.  

<a  name="json_parser"></a>  
## JSON Parser  
  
<a  name="spec_to_specs"></a>  
 
## Spec to specs  
  
Ο τρόπος με τον οποίο συνδέουμε τα στοιχεία των κλικών μεταξύ τους, είναι μέσω μίας δενδρικής αναπαράστασης. Το πρώτο spec που θα μπει στην κλίκα είναι ο πατέρας, ενώ τα υπόλοιπα είναι τα παιδιά του. Αν δύο κλίκες γίνουν **merge**, τότε ο πατέρας της δεύτερης κλίκας αποθηκεύεται στον παππού της πρώτης. Με αυτό τον τρόπο κρατάμε το δένδρο σε χαμηλά επίπεδα.  
  
To spec to specs είναι η δομή που χρησιμοποιούμε για το ζητούμενο της άσκησης. Στην ουσία είναι ένα hash table, στο οποίο το κάθε στοιχείο που αποθηκεύεται είναι της μορφής:  
```c  
  
struct SpecEntry_s {  
	  char *id;   
	  char *parent;
	  StrList *similar, *similar_tail, *different, *different_tail;   
	  ulong similar_len, different_len;  
	  bool printed;  
}; 
  
```  
Ο parent είναι ο πατέρας του spec (αν δεν είναι σε κλίκα τότε έχει για πατέρα τον εαυτό του). Η similar είναι μία λίστα στην οποία, αν  το spec είναι parent κάποιας κλίκας αποθηκεύονται τα ids όλης της κλίκας, σε αντίθετη περίπτωση της ανατίθεται η τιμή NULL, ενώ το similar_tail είναι  δείκτης στο τελευταίο στοιχείο της similar. Το similar_len είναι το μέγεθος της similar.  
  
Για τον υπολογισμό των διαφορετικών κλικών (different) βάλαμε άλλη μία λίστα στην οποία βρίσκονται οι root κάθε κλίκας που είναι διαφορετική. Αν για παράδειγμα είχαμε τη συσχέτιση "Α, Β, 0", ο root του Α θα συμπεριλάμβανε το root του Β στη different λίστα του, όπως και ο root του Β θα συμπεριλάμβανε το root του Α στη δική του different λίστα.

<a  name="machine_learning"></a>
## Μηχανική Μάθηση

Αρχικά έπρεπε να δημιουργηθέι το **vocabulary** σύμφωνα με το οποίο θα φτιάχνονταν τα διανύσματα για να γίνει το **training** και το **testing**. Έτσι δημιουργήσαμε το vocabulary με όλες τις διαφορετικές λέξεις των JSON αρχείων.
 
Για να μειωθούν οι διαστάσεις δεν συμπεριλάβαμε **stopwords**, punctuations και αριθμούς, όπως και κάποιες λέξεις που θεωρούμε πως δεν φέρουν χρήσιμη πληροφορία αφού υπάρχουν σε σχεδόν όλα τα αρχεία (πχ το "mm"). Στη συνέχεια αν ο χρήστης έχει δώσει την επιλογή **tfidf**, αφαιρούμε απο το vocabulary τις λέξεις με μεγάλο **idf**. 
Για να γίνει η διαδικασία επιλογής λέξεων δοκιμάσαμε τις παρακάτω μεθόδους: 
 - Η πρώτη μέθοδος ήταν να χρησιμοποιήσουμε συναρτήσεις όπως η **strtok** και **strcat**.
 - H δεύτερη ήταν να τροποποιήσουμε τον **tokenizer** που έχουμε υλοποιήσει για τον **json parser** και να υλοποιήσουμε την συνάρτηση **tokenizer_nlp_sw()** που αναλαμβάνει να επιστρέψει μόνο τις λέξεις που πληρούν τα κριτήριά αγνοώντας τις υπόλοιπες.

Παρατηρήσαμε πως η δεύτερη μέθοδος ήταν πιο γρήγορη οπότε και την προτιμήσαμε για το τελικό μας πρόγραμμα. Παρ' όλα αυτά υπάρχουν και οι δύο υλοποιήσεις καθώς και τα test τους στο repository.

Στη συνέχεια φτιάξαμε το **διάνυσμα** για το κάθε json και τα αποθηκεύσαμε σε ένα hash table για να τα έχουμε έτοιμα όταν τα χρειαστούμε. 

Για να δημιουργηθεί το dataset για το train, test και validation ακολουθήθηκε η εξής διαδικασία:

Αρχικά δημιουργήθηκαν όλα τα similar και different ζευγάρια μέσω των κλικών. Αυτό ήταν το συνολικό dataset. Στη συνέχεια κρατήσαμε τυχαία το **50% των similar** και το **50% των different** και τα ενώσαμε σε ένα πίνακα. Αυτός ο πίνακας αποτελεί το train set. Κάναμε την ίδια διαδικασία με **25% για το test set** και τα υπόλοιπα **25% για το validation set**. Έτσι είχαμε τα τρία sets που χρειαζόμασταν. 


### Εκπαίδευση μοντέλου
Για την εκπαίδευση υλοποιήσαμε mini **batch gradient decent**. 
Σε κάθε **epoch** αρχικά γίνεται  training για το training set και στη συνέχεια γίνεται predict για το test set ωστε να υπολογίζουμε το **max loss** σε κάθε **epoch**, με το σκεπτικό αν παρατηρήσουμε συνεχή αύξηση του max loss για 5 συνεχόμενα epochs, να κρατήσουμε το μοντέλο που είχε υπολογιστεί πριν 5 epochs και να σταματήσουμε τη διαδικασία του **training**. Παρατηρήσασμε όμως πως γινόταν **overfit** στα πρώτα epochs και έτσι σταματούσε το training στο πέμπτο epoch. Γι αυτό το λόγο κάναμε comment out το check και η εκπαίδευση γίνεται για όσα epochs έχουμε ορίσει από την αρχη με τον κίνδυνο να γίνει overfit για τα δεδομένα του training set. 

Στο τέλος του training κάνουμε **predict** για το validation set, τυπώνουμε τα predictions (με κόκκινο χρώμα τα λανθασμένα και με πράσινο τα σωστά) και στη συνέχεια υπολογίζουμε το **F1 score**

Παρατηρήσαμε ότι για **170 epochs**, **0,0001 learning rate** και **batch size 2000** παίρνουμε αρκετά καλό score της τάξης του **0,77**

### Εξαγωγή μοντέλου και λεξιλογίου σε αρχεία

Έχει υλοποιηθεί και δεύτερη main (**user.c**) η οποία κάνει μόνο predict ότι δώσει ο χρήστης σαν είσοδο. Χρησιμοποιόντας το vocabulary και το model που έχουν γίνει export από την πρώτη main. Για το λόγο αυτό το vocabulary και το τελικό model γίνονται export σε αρχεία ώστε να δοθούν ως είσοδο σε αυτήν.

### Επαναληπτική εκμάθηση

Στη συνέχεια έπρεπε να γίνει η διαδικασία της επαναληπτικής μάθησης την οποία είχαμε ξεκινήσει να υλοποιούμε στο branch **repetitive learning**, όμως δεν προλάβαμε να το ολοκληρώσουμε ώστε να το κάνουμε merge στο master. 

Η διαδικασία που ακολουθούσαμε ήταν να χρησιμοποιήσουμε όλο το dataset για  training (**train + test + val sets**) και για threshold είχαμε 0.15. Για όλα τα υπόλοιπα πιθανά ζευγάρια (δηλαδή όλα τα ζευγάρια απο specs για τα οποία δεν ξέρουμε τη σχέση τους, κάναμε predict. Aν το μοντέλο μας ήταν αρκετά "σίγουρο" προσθέταμε όλες τις συσχετίσεις που προέκυπταν από αυτό το prediction. Συνεχίζαμε τη διαδικασία για όσο το **threshold**  ήταν < **0,5**.

  <a  name="parallel"></a>  
## Παραλληλοποίηση
Για την γρήγορότερη εκπαίδευση του μοντέλου υλοποιήθηκε ένας **Job Scheduler** όπου αναλαμβάνει να τρέξει Jobs σε διαφορετικά νήματα του συστήματος, προσθέτοντας ετσι παραλληλία.
### Structs
*Αναπαράσταση της δομής job:*
```c
struct job {
	long long int job_id;  
	void *(*start_routine)(void *);
	int args_count;  
	Argument *args;  
	void *return_val;  
	bool complete;  
	/* sync */  
	sem_t sem_complete;  
};
```
Κάθε νέο **job** που δημιουργείται μέσω της συνάρησης **js_create_job()** έχει τα παρακάτω χαρακτηριστικά:
- μοναδικό id (**job_id**)
- δείκτη σε συνάρτηση (**start_routine**) για την ρουτίνα εκτέλεσης
- πίνακα με τα ορίσματα της ρουτίνας (**args**)
- τιμή επιστροφής της ρουτίνας (**return_val**)
- flag (**complete**) που υποδηλώνει αν το job ολοκληρώθηκε ή όχι
- semaphore (**sem_complete**) ώστε αν κάποιο νήμα ζητήσει να δει την τιμή επιστροφής του, να είναι εφικτό να μπλοκάρει μέχρι αυτο να ολοκληρωθεί. 

*Αναπαράσταση της δομής job_scheduler:*
```c
struct job_scheduler {  
	 uint execution_threads;  
	 pthread_t *tids;  
	 Queue waiting_queue;  
	 Queue running_queue;  
	 bool working;  
	 bool exit;  
	 int ready;
	 /* sync */   
	 pthread_cond_t condition_wake_up;  
	 pthread_cond_t condition_wake_up_submitter;  
	 pthread_mutex_t mutex;  
	 pthread_mutex_t mutex_submitter;  
	 sem_t_ *sem_barrier;  
};
```
Η δομή του Job Scheduler αποτελείται έχει τα παρακάτω χαρακτηριστικά:
- Αριθμός διαθέσιμων νημάτων (**execution_threads**) για εκτέλεση εργασιών.
- Πίνακας με τα αναγνωριστικά των νημάτων (**tids**) που χρησιμοποιείται για την ευρεσή τους με σκοπό την ένωσή τους με το αρχικό νήμα κανωντας κλήση της σηνάρτησης **pthread_join**.
- Ουρά αναμονής (**waiting_queue**) προς εκτέλεση εργασιών.
- Ουρά εργασιών που ξεκίνησαν να τρέχουν (**running_queue**) η οποία χρησιμοποιείται για να τα συλλέξει με τη σωστή σειρά το master thread όταν θα έχουν ολοκληρωθεί όλα.
- Flag  (**working**) που χρησιμοποιείται για να υποδηλώσει ένα ενεργό **work cycle** όταν έχει την τιμή true και **barrier ready** όταν έχει την τιμή false.
- Flag (**exit**) που υποδηλώνει ότι το master thread αποφάσισε να μην χρησιμοποιήσει άλλο τον scheduler. Αν λάβει την τιμή true, τοτε όταν το κάθε νήμα "ξυπνήσει" θα καλέσει την **pthread_exit**.
- Μετρητής έτοιμων νημάτων (**ready**) που δεν τρέχουν κάποια εργασία αυτή τη στιγμή.
ενώ για τον συνχρωνισμό:
- Mutex  (**mutex**)  που χρησιμοποιείται για τον αμοιβαίο αποκλεισμό των άλλων νημάτων στις περιπτώσεις που χρειάζεται.
- Condition variable (**condition_wake_up**) που χρησιμοποιείται για να μπλοκάρουν τα νήματα που είναι έτοιμα και δεν υπάρχει ακόμα διαθέσιμο job.
- Mutex αποστολέα  (**mutex_submitter**) για τον αμοιβαίο αποκλεισμό των υπόλοιπων νημάτων που προσπαθούν εκείνη τη στιγμή ταυτόχρονα να βάλουν μία νέα εργασία στην ουρά.
- Condition variable (**condition_wake_up_submitter**) που χρησιμοποιείται για να μπλοκάρει μέχρι να υπάρξει κάποιο διαθσιμο νήμα.
- Custom semaphore (**sem_barrier**) που χρησιμοποιείται για να ενημερώσει το master thread που έχει καλέσει την **join_threads** ότι όλα τα νήματα είναι έτοιμα. 

### Thread pool
Ο Job scheduler αναλαμβάνει να τρέξει εργασίες (jobs) σε διαφορετικά νήματα με FIFO (First In First Out) σειρά και αυτό το επιτυγχάνει με μια ουρά αναμονής (waiting_queue). 
Όταν γίνεται εξαγωγή ενώς job από την waiting_queue το πρώτο διαθέσιμο νήμα που 

### Work cycle
Όταν ξεκινάει να λειτουργεί ένα νήμα, κατευθείαν μπαίνει σε ατέρμον επανάλληψη έως ότου το master thread αποφασίσει να καταστρέψει τον scheduler. Στην αρχή κάθε επαννάληψης της κεντρικής λούπας κλειδώνει 

ελέγχει αν ο scheduler είναι σε κατάσταση working ή exit 

αναμονής μέχρι να υπάρξει διαθέσιμη εργασία.


```c
void *thread(JobScheduler js) {
    int jobs_count = 0;
    while (true) {
        LOCK_;
        while ((!js->working && !js->exit) || (!queue_size(js->waiting_queue) && !js->exit)) {
            js->ready++;
            if (js->ready == js->execution_threads) {
                js->working = false;
            }
            pthread_cond_signal(&js->condition_wake_up_submitter);
            NOTIFY_BARRIER_;
            WAIT_;
            js->ready--;
        }
        if (js->exit && !queue_size(js->waiting_queue)) {
            UNLOCK_;
            EXIT_;
        }
        Job job = NULL;
        if (queue_dequeue(js->waiting_queue, &job, false)) {
            jobs_count++;
            queue_enqueue(js->running_queue, &job, false);
            queue_unblock_enqueue(js->waiting_queue);
            UNLOCK_;
            RUN_ROUTINE_;
            job->complete = true;
            NOTIFY_JOB_COMPLETE_;
            continue;
        }
        UNLOCK_;
    }
}
```



<a  name="flow"></a>  
## Ροή του προγράμματος
 ![cliques](https://raw.githubusercontent.com/vasilisp100/k23a-project/master/resources/cliques.gif?token=AMOC6I6TSAEO3RWM4E22FUK7Z7ZZS)  
  
<a  name="unit_tests"></a>  
## Unit tests  

Κάθε module (**list**, **hash table**, **hset**, **job_scheduler**, **json_parser**, **ml**, **logreg**, **queue**, κτλ) έχει αντίστοιχα και τα δικά του unit tests τα οποία βρίσκονται σε διαφορετικά αρχεία το καθένα. Για την υλοποίησή τους, κάνουμε χρήση της βιβλιοθήκης [Acutest](https://github.com/mity/acutest).
Όταν επιχειρούμε να κάνουμε complile (make), commit  ή push εκτελούνται τα unit tests και αν τερματίσουν επιτυχώς, πραγματοποιείται με επιτυχία η αντίστοιχη διαδικασία.

Επιπλέον, τα test αυτά εκτελούνται remotely μέσω των github actions (CI)

<a  name="documentation"></a>  
## Documentation

Στον σύνδεσμο [Documentation](https://k23a-prj-auth.herokuapp.com) που βρίσκεται και στην πρώτη σελίδα του report βρίσκεται το documentation (man pages) των βιβλιοθηκών που έχουν υλοποιηθεί. Πρόσβαση επιτρέπεται μόνο στους contributors του project (δηλαδή την ομάδα μας και το βοηθό του τμήματός μας) και πρέπει να γίνει authentication μέσω github, ωστε να μην μπορεί κάποιος άλλος να αποκτήσει πρόσβαση.

Το Documentation υλοποιήθηκε μέσω του [Doxygen](https://www.doxygen.nl/index.html). Τα man pages αυτά ανανεώνονται κάθε φορά που προστίθεται νέο commit στο master. Τα output αρχεία του doxygen γίνονται push στο branch "**docs**" και απο εκεί ενημερώνεται το documentation site μας.

<a  name="conclusions"></a>  
## Συμπεράσματα

