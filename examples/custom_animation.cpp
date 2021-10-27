#include <dmsc/animation.hpp>
#include <dmsc/glm_include.hpp>
#include <dmsc/visuals.hpp>

int main() {
    dmsc::Instance instance;

    // 1. create satellites
    dmsc::StateVector sv; // represents a satellite
    sv.height_perigee = 200.f;
    sv.cone_angle = dmsc::rad(20.f);
    instance.satellites.push_back(sv); // satellite 0
    sv.initial_true_anomaly = 0.1f;
    sv.inclination = dmsc::rad(45.f);
    sv.rotation_speed = 0.05f;
    instance.satellites.push_back(sv); // satellite 1
    sv.inclination = dmsc::rad(23.f);
    instance.satellites.push_back(sv); // satellite 2

    // 2. define which satellites can communicate with each other
    instance.edges.push_back(dmsc::Edge(0, 1)); // link between sat 0 and sat 1
    instance.edges.push_back(dmsc::Edge(0, 2)); // link between sat 0 and sat 2
    instance.edges.push_back(dmsc::Edge(1, 2)); // link between sat 1 and sat 2

    // 3. schedule directed communications between two satellites
    instance.edges.push_back(dmsc::Edge(0, 1, dmsc::EdgeType::SCHEDULED_COMMUNICATION));

    // 4. create custom animation
    dmsc::Animation animation;
    animation.addSatelliteAnimation(1, 65, 70, dmsc::AnimationDetails(false)); // hide sat. with idx 1 from t=65 to
    // t=70
    animation.addSatelliteAnimation(
        2,
        55,
        80,
        dmsc::AnimationDetails(true, glm::vec4(0.8f, 0.23f, 1, 1))); // hide sat. with idx 1 from t=65 to t=70
    // the isl with idx 0 if colored blue from t=60 to t =70
    animation.addSatelliteAnimation(0, 55, 60, dmsc::AnimationDetails(true, glm::vec4(0.3f, 0.23f, 0.62f, 1)));
    animation.addSatelliteAnimation(0, 60, 65, dmsc::AnimationDetails(true, glm::vec4(0.9f, 0.23f, 0.1f, 1)));
    animation.addSatelliteAnimation(0, 65, 70, dmsc::AnimationDetails(true, glm::vec4(0.7f, 0.8f, 0.5f, 1)));
    animation.addISLAnimation(0, 60, 70, dmsc::AnimationDetails(true, glm::vec4(0.f, 0.f, 1.f, 1.f)));
    animation.addISLAnimation(1, 55, 70, dmsc::AnimationDetails(false));
    animation.addOrientationAnimation(1, 40, dmsc::OrientationDetails(glm::vec3(0,1,0)));
    animation.addOrientationAnimation(1, 6000, dmsc::OrientationDetails(glm::vec3(0.1f,-0.7f,0), 0.1f));

    // 5. visualize instance and animation
    dmsc::visualizeCustom(instance, animation, 50);

    return 0;
}
