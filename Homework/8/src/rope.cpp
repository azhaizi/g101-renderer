#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.
        for(int i=0;i<num_nodes;i++){
            float t = num_nodes<1 ? 0.0f : (float)i / (num_nodes-1);
            Vector2D pos = start * (1-t) + end * t;
            masses.push_back(new Mass(pos,node_mass,false));
        }
//        Comment-in this part when you implement the constructor
        for (auto &i : pinned_nodes) {
            masses[i]->pinned = true;
        }

        for(int i=0;i<num_nodes-1;i++){
            springs.push_back(new Spring(masses[i],masses[i+1],k));
        }

    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            Vector2D dir = s->m1->position-s->m2->position;
            double dirlen = dir.norm();
            if(dirlen<1e-8) continue;

            Vector2D a = -1 * s->k * (dir / dirlen) * (dirlen - s->rest_length);

            s->m1->forces += a;
            s->m2->forces -= a;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                m->forces += gravity * m->mass;
                Vector2D a = m->forces / m->mass;

                m->velocity += a * delta_t;           
                m->position += m->velocity * delta_t;//半隐式

                // m->position += m->velocity * delta_t;  //显式
                // m->velocity += a * delta_t;

                m->velocity *= (1.0f - 0.001f);
                // TODO (Part 2): Add global damping
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            Vector2D dir = s->m2->position-s->m1->position;
            double dirlen = dir.norm();
            if(dirlen<1e-8) continue;

            Vector2D a = dir  * ((dirlen - s->rest_length)/ dirlen);

            if(!s->m1->pinned) s->m1->position += a * 0.5f;
            if(!s->m2->pinned) s->m2->position -= a * 0.5f;
        }

        double damping_factor = 0.00005f;

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                Vector2D temp_position = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
                m->forces += gravity * m->mass;
                Vector2D a = m->forces / m->mass;

                m->position += (1 - damping_factor) * (m->position - m->last_position) + a * delta_t *delta_t;

                m->last_position = temp_position;
                // TODO (Part 4): Add global Verlet damping
            }
            m->forces = Vector2D(0,0);
        }
    }
}
