
  
# Εθνικό και Καποδιστριακό Πανεπιστήμιο Αθηνών  
### Τμήμα Πληροφορικής και Τηλεπικοινωνιών  
### Κ23Α - ΑΝΑΠΤΥΞΗ ΛΟΓΙΣΜΙΚΟΥ ΓΙΑ ΠΛΗΡΟΦΟΡΙΑΚΑ ΣΥΣΤΗΜΑΤΑ  
### **Project - Entity resolution**  
  
**Μέλη**:  
 - Θεόδωρος Χατζηιωαννίδης - 1115201600197  
 - Βασίλειος Πουλόπουλος - 1115201600141  
 - Κωνσταντίνος Χατζόπουλος - 1115201300202  

## Περιεχόμενα 

1. [Εισαγωγή](#intro)
2. [Hash table](#hash_table)
3. [List](#list)
4. [JSON Parser](#json_parser)
5. [Spec to specs](#spec_to_specs)
6. [Ροή του προγράμματος](#flow)
7. [Unit tests](#unit_tests)
8. [Συμπεράσματα](#conclusions)

<a name="intro"></a>
## Εισαγωγή  
 Στόχος της άσκησης είναι να δημιουργήσουμε κλίκες γράφων από αγγελίες με τα ίδια προϊόντα σε διάφορα site. Για την υλοποίηση χρειάστηκε να δημιουργήσουμε τις παρακάτω δομές δεδομένων:
 
 - **hash table**:   
 - **list**

Επίσης, υλοποιήσαμε έναν json parser ώστε να συλλέξουμε τα δεδομένα των αγγελιών από τα json αρχεία.

Επιπλέον, για τον έλεγχο ορθότητας του κώδικα, χρησιμοποιήσαμε τη βιβλιοθήκη **acutest.h** όπου δέχεται μια λίστα από κατάλληλες συναρτήσεις (tests) που γράψαμε.

 
 <a name="hash_table"></a>
## Hash table  

![enter image description here](https://raw.githubusercontent.com/vasilisp100/k23a-project/master/resources/hash.png?token=AMOC6I6CKFXV7YQAJ5V36Q27YVF5U)

Για την υλοποίηση του hash table, χρησιμοποιήσαμε generic **open addressing hash table** με **random probing**.

Πιο συγκεκριμένα, δεσμεύουμε ένα κομμάτι μνήμης το οποίο χωρίζεται  σε μκρότερα ίσα μέρη ώστε στα οποία αποθηκεύονται οι πληροφορίες του κάθε **spec**. 

Αφού χρησιμοποιούμε random probing το κάθε spec ανάλογα με το key του, αν δε χωράει στην πρώτη θέση που θα υπολογιστεί μέσω του hash function, θα ψάξει κάποια "τυχαία" θέση για να αποθηκευθεί. Το seed της random συνάρτησης είναι σταθερό, οπότε πάντα μπορούμε να υπολογίσουμε σε ποιο σημείο της μνήμης βρίσκονται το εκάστοτε δεδομένα.

Αν η μνήμη που έχουμε δεσμεύσει αρχίσει να γεμίζει θα κάνουμε **Rehashing**. 
Όταν θέλουμε να εισάγουμε νέα δεδομένα αλλά η πληρότητα του είναι 70%, δεσμεύουμε τη διπλάσια μνήμη από πριν, ξαναμοιράζουμε τα δεδομένα στην καινούργια, μεγαλύτερη μνήμη και εισάγουμε τα νέα δεδομένα. 

Η δομή που χρησιμοποιήσαμε για το hash table είναι η παρακάτω: 


```c
typedef struct htab_s {
	    /*! @brief hash function used to hash the keys */
	    ht_hash_func h;
	    /*! @brief comparison function to compare 2 keys (default: memcmp) */
	    ht_cmp_func cmp;
	    /*! @brief copying function that copies a key to the hashtable (default:
	    * memcpy) */
	    ht_key_cpy_func keycpy;
	    /*! @brief size of key in the hashtable */
	    size_t key_sz;
	    /*! @brief size of val in the hashtable */
	    size_t val_sz;
	    /*! @brief capacity of buf */
	    ulong buf_cap;
	    /*! @brief occupied entries of buf */
	    ulong buf_load;
	    /*! @brief the buffer where the entries are stored */
	    char buf[];
    } htab_t;
 ```



  
 <a name="list"></a>
## List  
Χρησιμοποιήσαμε μία generic **list**.  




 <a name="json_parser"></a>
## JSON Parser  





 <a name="spec_to_specs"></a>
## Spec to specs  

Ο τρόπος με τον οποίο συνδέουμε τα στοιχεία των κλικών μεταξύ τους, είναι μέσω μίας δενδρικής αναπαράστασης. Το πρώτο spec που θα μπει στην κλίκα είναι ο πατέρας, ενώ τα υπόλοιπα είναι τα παιδιά του. Αν δύο κλίκες γίνουν **merge**, τότε ο πατέρας της δεύτερης κλίκας πάει και αποθηκεύεται στον παππού της πρώτης. Με αυτό τον τρόπο κρατάμε το δένδρο σε χαμηλά επίπεδα.

To spec to specs είναι η δομή που χρησιμοποιούμε για το ζητούμενο της άσκησης. Στην ουσία είναι ένα hash table, στο οποίο το κάθε στοιχείο που αποθηκεύεται είναι της μορφής: 
```c
struct SpecEntry_s {
	
	/*! @brief spec id */
	char *id;
	/*! @brief Set of similar specs. */
	char *parent;
	/*! @brief Contents of the set
	if this node is the representative of the set, this is the list of the elements;
	otherwise, this is NULL
	*/
	StrList *similar, *similar_tail;
	/*! @brief Length of similar */
	ulong similar_len;
};
```
Ο parent είναι ο πατέρας του spec (αν δεν είναι σε κλίκα είναι ο εαυτός του) . Η similar είναι μία λίστα στην οποία, αν το spec είναι  parent κάποιας κλίκας αποθηκεύονται τα ids όλης της κλίκας αλλιώς είναι NULL, ενώ το similar_tail είναι δείκτης στο τελευταίο στοιχείο της similar. Το similar_len είναι το μέγεθος της similar.





 <a name="flow"></a>
## Ροή του προγράμματος 



  
 <a name="unit_tests"></a>
## Unit tests  




  
 <a name="conclusions"></a>
## Συμπεράσματα  
