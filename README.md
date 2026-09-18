Este es un motor 2D basado en raylib.

Arquitectura:
- Actor representa entidades del juego.
- Tiene ActorManager para gestionar la creación y vida de los actores.
- AI Controller gestiona unicamente los labels de los AIBehaviours (enum EnemyBehavior)
- ActorFactory implementa esos EnemyBehaviours
- CollisionActors y CollisionTilemaps tiene las formulas para colisiones
entre distintos tipos (AABB)
- PhysicsSystem tiene las formulas separadas de los elementos fisicos: gravedad, carrying, etc.
- collider.h declara el struct Collider basico
- VFX gestiona entidades graficas temporales

This is a 2D engine based on raylib.

Architecture:

* Actor represents game entities.
* It has an ActorManager to handle the creation and lifecycle of actors.
* The AI Controller only manages the labels of the AIBehaviours (enum EnemyBehavior).
* ActorFactory implements those EnemyBehaviours.
* CollisionActors and CollisionTilemaps contain the collision formulas between different types (AABB).
* PhysicsSystem contains the formulas separated from the physical elements: gravity, carrying, etc.
* collider.h declares the basic Collider struct.
* VFX manages temporary graphical entities.
