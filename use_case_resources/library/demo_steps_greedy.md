# Steps for library demo

ITEM_7 -> si sposta il robot 5

---

ITEM_1 -> si sposta il robot 1

---

ITEM_2 -> viene cliccato a metà del successivo quadrante

cosa succede? il robot 2 interromperà il proprio spostamento, mentre il robot 1 si sposterà verso ITEM_2

---

caso out of order: 
ITEM_5 -> si sposta il robot 5, ma dopo essere partito, viene invocato (manualmente) lo script bash out_of_order per simulare la rottura del robot: verrà bloccato a metà strada e lo andranno a soccorrere gli altri robot (probabilmente il robot1).

---
ITEM_1 -> per far muovere il robot2

----

DOCK_1 -> per far vedere le doti di movimento del robot in gazebo. lo interrompo appena prima di arrivare per simulare l'abort di un goal.

---
far passare un po' di tempo in modo che il robot1 abbia (strettamente) meno del 90% di batteria.
ITEM_8 -> non si muove il robot1, perchè è al 89%, ma si muove il robot4

---

DOCK_3 -> in modo che il robot3 si infili sotto il tavolo

---

DOCK_5 -> il robot 3 si sposta tra le sedie
