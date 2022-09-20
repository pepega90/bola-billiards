#include <raylib.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <algorithm>

const int WIDTH = 900;
const int HEIGHT = 480;

struct Body
{
    Vector2 pos;
    Vector2 vel;
    Vector2 acc;
    Color warna;

    float radius;
    float mass;
    float invMass;
    float restituion;

    bool collide = false;

    Body(float x, float y, float radius, Color warna, float mass)
    {
        this->pos.x = x;
        this->pos.y = y;
        this->radius = radius;
        this->mass = mass;
        this->warna = warna;
        this->restituion = 1.0;
        if (this->mass != 0.0)
        {
            this->invMass = 1.0 / mass;
        }
        else
        {
            this->invMass = 0.0;
        }
    }

    bool IsStatic()
    {
        const float epsilon = 0.005f;
        return fabs(invMass - 0.0) < epsilon;
    }

    void applyImpulse(Vector2 j)
    {
        if (IsStatic())
            return;

        vel += j * invMass;
    }
};

// struct Contact, menyimpan informasi ketika terjadi collision
struct Contact
{
    Body *a;
    Body *b;

    Vector2 start;
    Vector2 end;

    Vector2 normal;
    float depth;

    Contact() = default;
    ~Contact() = default;

    // projection method
    void ResolvePenetration();
    // impulse method
    void ResolveCollision();
};

void Contact::ResolvePenetration()
{
    float da = depth / (a->invMass + b->invMass) * a->invMass;
    float db = depth / (a->invMass + b->invMass) * b->invMass;

    a->pos -= normal * da;
    b->pos += normal * db;
}

void Contact::ResolveCollision()
{
    ResolvePenetration();

    float e = std::min(a->restituion, b->restituion);

    Vector2 vrel = (a->vel - b->vel);

    float vRelDotNormal = vrel.Dot(normal);

    // kalkulasi impulse
    Vector2 impulseDirection = normal;
    float impulseMagnitude = -(1 + e) * vRelDotNormal / (a->invMass + b->invMass);

    // hasil impulse
    Vector2 j = impulseDirection * impulseMagnitude;

    // apply impulse
    a->applyImpulse(j);
    b->applyImpulse(-j);
}

bool IsCollide(Body *a, Body *b, Contact &contact)
{

    const Vector2 ab = b->pos - a->pos;
    const float radiusSum = a->radius + b->radius;

    bool isColliding = ab.MagnitudeSquared() <= (radiusSum * radiusSum);

    if (!isColliding)
    {
        return false;
    }

    contact.a = a;
    contact.b = b;

    contact.normal = ab;
    contact.normal.Normalize();

    contact.start = b->pos - contact.normal * b->radius;
    contact.end = a->pos + contact.normal * a->radius;

    contact.depth = (contact.end - contact.start).Magnitude();

    return true;
}

void reset(std::vector<Body *> &bodies)
{
    // bikin segitiga bola
    Body *b1 = new Body(633, 244, 15, BLACK, 1.0);
    Body *b2 = new Body(654, 234, 15, YELLOW, 1.0);
    Body *b3 = new Body(654, 259, 15, BLUE, 1.0);
    Body *b4 = new Body(675, 216, 15, MAGENTA, 1.0);
    Body *b5 = new Body(677, 239, 15, PURPLE, 1.0);
    Body *b6 = new Body(675, 269, 15, PINK, 1.0);
    Body *b7 = new Body(699, 195, 15, BEIGE, 1.0);
    Body *b8 = new Body(700, 223, 15, GOLD, 1.0);
    Body *b9 = new Body(703, 252, 15, ORANGE, 1.0);
    Body *b10 = new Body(701, 285, 15, VIOLET, 1.0);
    Body *b11 = new Body(719, 185, 15, DARKPURPLE, 1.0);
    Body *b12 = new Body(723, 210, 15, SKYBLUE, 1.0);
    Body *b13 = new Body(726, 234, 15, RED, 1.0);
    Body *b14 = new Body(728, 267, 15, DARKGRAY, 1.0);
    Body *b15 = new Body(730, 294, 15, GRAY, 1.0);
    bodies.push_back(b1);
    bodies.push_back(b2);
    bodies.push_back(b3);
    bodies.push_back(b4);
    bodies.push_back(b5);
    bodies.push_back(b6);
    bodies.push_back(b7);
    bodies.push_back(b8);
    bodies.push_back(b9);
    bodies.push_back(b10);
    bodies.push_back(b11);
    bodies.push_back(b12);
    bodies.push_back(b13);
    bodies.push_back(b14);
    bodies.push_back(b15);
}

void collisionWithPocket(Body *ball, std::vector<Vector2> &pockets, int &index, std::vector<Body *> &bodies, Sound &sfx)
{
    for (int i = 0; i < pockets.size(); i++)
    {
        if (CheckCollisionCircles(ball->pos, ball->radius, pockets[i], 20.0) && index == 0)
        {
            ball->pos = Vector2(GetScreenWidth() / 4.0f, 250);
            ball->acc, ball->vel = Vector2();
            bodies.erase(bodies.begin() + 1, bodies.end());
            reset(bodies);
        }
        else if (CheckCollisionCircles(ball->pos, ball->radius, pockets[i], 20.0) && index != 0)
        {
            PlaySound(sfx);
            ball->collide = true;
        }
    }
}

int main()
{
    InitWindow(WIDTH, HEIGHT, "Practice");
    InitAudioDevice();
    SetTargetFPS(60);

    // load sfx
    Sound ballCollideSfx = LoadSound("./assets/BallsCollide.wav");
    Sound holeSfx = LoadSound("./assets/Hole.wav");

    bool press = false;
    bool bounce = false;
    float bounceLoss = 0.9;
    Vector2 mousePos;
    Color listColor[] = {YELLOW, BLACK, ORANGE, PINK, RED, BLUE, PURPLE};

    // Body *bigBall = new Body(GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f, 100, 0.0);
    Body *b0 = new Body(GetScreenWidth() / 4.0f, 250, 15, WHITE, 1.0);
    // list of ball
    std::vector<Body *> bodies;
    bodies.push_back(b0);

    // table
    Rectangle table = Rectangle{0.0, 0.0, WIDTH, HEIGHT};
    float borderTable = 40.0;

    // pocket
    std::vector<Vector2> pockets;
    float pocketRadius = 30.0;
    // top
    pockets.push_back(Vector2(35, 35));
    pockets.push_back(Vector2(GetScreenWidth() / 2.0f, 35));
    pockets.push_back(Vector2(GetScreenWidth() - 35.0, 35));
    // bottom
    pockets.push_back(Vector2(35, GetScreenHeight() - 35.0));
    pockets.push_back(Vector2(GetScreenWidth() / 2.0f, GetScreenHeight() - 35.0));
    pockets.push_back(Vector2(GetScreenWidth() - 35.0, GetScreenHeight() - 35.0));

    reset(bodies);

    while (!WindowShouldClose())
    {

        ClearBackground(DARKGREEN);
        BeginDrawing();

        float dt = GetFrameTime();

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            mousePos = GetMousePosition();
            press = true;
        }

        if (IsMouseButtonUp(MOUSE_BUTTON_LEFT) && press)
        {
            press = false;
            Vector2 impulseDir = (bodies[0]->pos - mousePos).UnitVector();
            float impulseMagnitude = (bodies[0]->pos - mousePos).Magnitude() * 20.0;
            bodies[0]->vel = impulseDir * impulseMagnitude;
        }

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            Vector2 mousePos = GetMousePosition();
            Body *b = new Body(mousePos.x, mousePos.y, 15, listColor[GetRandomValue(0, 6)], 1.0);
            bodies.push_back(b);
        }

        for (auto &b : bodies)
        {

            if (!b->IsStatic())
            {

                b->acc = -b->vel * 0.8;

                b->vel += b->acc * dt;

                b->pos += b->vel * dt;
            }
        }

        for (auto &body : bodies)
        {
            if (body->pos.x - body->radius <= borderTable)
            {
                body->pos.x = borderTable + body->radius;
                body->vel.x *= -0.9;
            }
            else if (body->pos.x + body->radius >= table.width - borderTable)
            {
                body->pos.x = table.width - body->radius - borderTable;
                body->vel.x *= -0.9;
            }
            if (body->pos.y - body->radius <= borderTable)
            {
                body->pos.y = borderTable + body->radius;
                body->vel.y *= -0.9;
            }
            else if (body->pos.y + body->radius >= table.height - borderTable)
            {
                body->pos.y = table.height - body->radius - borderTable;
                body->vel.y *= -0.9;
            }
        }

        for (int i = 0; i <= bodies.size() - 1; i++)
        {
            for (int j = i + 1; j < bodies.size(); j++)
            {
                Body *a = bodies[i];
                Body *b = bodies[j];
                Contact contact;
                if (IsCollide(a, b, contact))
                {
                    PlaySound(ballCollideSfx);
                    bounce = true;
                    contact.ResolveCollision();
                }
            }
        }

        // loss energy after collision
        if (bounce)
        {
            for (auto &b : bodies)
            {
                b->vel *= bounceLoss;
            }
            bounce = false;
        }

        // check collision bola dengan pocket
        for (int i = 0; i < bodies.size(); i++)
        {
            if (bodies.size() > 1)
                collisionWithPocket(bodies[i], pockets, i, bodies, holeSfx);
        }

        // remove ball if collide with pocket
        auto removedBall = std::remove_if(bodies.begin(), bodies.end(), [&](Body *b)
                                          { return b->collide == true; });
        if (removedBall != bodies.end())
            bodies.erase(removedBall);

        if (press)
        {
            DrawLineV(bodies[0]->pos, mousePos, YELLOW);
        }

        // draw table
        DrawRectangleLinesEx(table, borderTable, DARKBROWN);

        // draw pocket
        for (auto &p : pockets)
        {
            DrawCircleV(p, pocketRadius, BLACK);
        }

        for (int i = 0; i < bodies.size(); i++)
        {
            DrawCircleV(bodies[i]->pos, bodies[i]->radius, bodies[i]->warna);
        }

        EndDrawing();
    }
    UnloadSound(holeSfx);
    UnloadSound(ballCollideSfx);
    CloseWindow();

    return 0;
}