#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

const float GRAVITY = 9.8f;
const float TIME_STEP = 0.0167f; // 60 FPS
const float ELASTICITY = 0.8f; // Energy retained after collisions
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int NUM_PARTICLES = 10;

struct Particle {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    float radius;

    Particle(float x, float y, float r, const sf::Color &color) {
        radius = r;
        shape.setRadius(radius);
        shape.setFillColor(color);
        shape.setPosition(x, y);
        shape.setOrigin(radius, radius);
        velocity = sf::Vector2f(0, 10);
        acceleration = sf::Vector2f(0, GRAVITY);
    }

    void move(float dt) {
        velocity += acceleration * dt;
        shape.move(velocity * dt);
    }
};

void handleBoundaryCollision(Particle &particle) {
    sf::Vector2f position = particle.shape.getPosition();

    if (position.x - particle.radius < 0) {
        particle.velocity.x = -particle.velocity.x * ELASTICITY;
        particle.shape.setPosition(particle.radius, position.y);
    } else if (position.x + particle.radius > WINDOW_WIDTH) {
        particle.velocity.x = -particle.velocity.x * ELASTICITY;
        particle.shape.setPosition(WINDOW_WIDTH - particle.radius, position.y);
    }

    if (position.y - particle.radius < 0) {
        particle.velocity.y = -particle.velocity.y * ELASTICITY;
        particle.shape.setPosition(position.x, particle.radius);
    } else if (position.y + particle.radius > WINDOW_HEIGHT) {
        particle.velocity.y = -particle.velocity.y * ELASTICITY;
        particle.shape.setPosition(position.x, WINDOW_HEIGHT - particle.radius);
    }
}

void handleParticleCollision(Particle &p1, Particle &p2) {
    sf::Vector2f diff = p1.shape.getPosition() - p2.shape.getPosition();
    float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    float overlap = p1.radius + p2.radius - distance;

    if (distance < p1.radius + p2.radius) {
        sf::Vector2f normal = diff / distance;
        sf::Vector2f relativeVelocity = p1.velocity - p2.velocity;

        float velocityAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;
        if (velocityAlongNormal > 0) return;

        float restitution = ELASTICITY;
        float impulseScalar = -(1 + restitution) * velocityAlongNormal;
        impulseScalar /= 2; // assuming equal mass

        sf::Vector2f impulse = impulseScalar * normal;
        p1.velocity += impulse;
        p2.velocity -= impulse;

        // Separate particles to prevent sinking
        p1.shape.move(normal * (overlap / 2.0f));
        p2.shape.move(-normal * (overlap / 2.0f));
    }
}

int main() {
    srand(static_cast<unsigned>(time(0)));

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D Physics Simulator");
    window.setFramerateLimit(60);

    std::vector<Particle> particles;
    for (int i = 0; i < NUM_PARTICLES; ++i) {
        auto radius = static_cast<float>(rand() % 10 + 10);
        auto x = static_cast<float>(rand() % (WINDOW_WIDTH - 2 * static_cast<int>(radius)) + radius);
        auto y = static_cast<float>(rand() % (WINDOW_HEIGHT - 2 * static_cast<int>(radius)) + radius);
        sf::Color color(rand() % 256, rand() % 256, rand() % 256);
        particles.emplace_back(x, y, radius, color);
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // Update particles
        for (auto &particle : particles) {
            particle.move(TIME_STEP);
            handleBoundaryCollision(particle);
        }

        // Handle particle-particle collisions
        for (size_t i = 0; i < particles.size(); ++i) {
            for (size_t j = i + 1; j < particles.size(); ++j) {
                handleParticleCollision(particles[i], particles[j]);
            }
        }

        // Render
        window.clear(sf::Color::Black);
        for (const auto &particle : particles) {
            window.draw(particle.shape);
        }
        window.display();
    }

    return 0;
}
