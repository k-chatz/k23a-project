
  
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
  
1. [Εισαγωγή](#intro)  
  
2. [Hash table](#hash_table)  
  
3. [List](#list)  
  
4. [JSON Parser](#json_parser)  
  
5. [Spec to specs](#spec_to_specs)  
  
6. [Ροή του προγράμματος](#flow)  
  
7. [Unit tests](#unit_tests)  
  
8. [Συμπεράσματα](#conclusions)  
  
    
    
  
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
## Έξοδος προγράμματος Training  
  
    
  **predictions** και **f1 score** για το validation set  
  
  
## Έξοδος προγράμματος Training  
  
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