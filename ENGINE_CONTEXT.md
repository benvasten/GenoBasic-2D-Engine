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


Objetivo:
Añadir nuevas features sin romper compatibilidad.