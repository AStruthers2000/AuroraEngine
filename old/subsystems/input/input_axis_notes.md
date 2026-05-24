For the input subsystem, I want a few different "types" of inputs. 

There should be an "axis" type of input, that can create vectors of analog values. For example, a
0-d axis, 1-d axis, and 2-d axis. These axis representations aren't really vectors, more like a way
to merge multiple inputs. For example, in the OneDimensionAxis, let's say we have the up and down
arrows as trigger_p and trigger_n respectively. We would get the following states:

| Trigger State |   Vector    | Normalized |
|:-------------:|:-----------:|:----------:|
|    {0, 0}     | {0.f, 0.f}  |    0.f     |
|    {0, 1}     | {0.f, -1.f} |    -1.f    |
|    {1, 0}     | {1.f, 0.f}  |    1.f     |
|    {1, 1}     | {1.f, -1.f} |    0.f     |

Instead of combining both states into the normalized version directly (+/-1 to the value
respectively), it will sometimes be useful to have the original vector and the normalized value at
the same time. For example, in Trackmania if you press accelerate and break at the same time, the
result isn't a net 0 acceleration like in the case where both accelerate and break are depressed,
the car still accelerates _and_ brakes, aka drifting. If we just cared about the normalized value,
we would miss crucial information. Likewise in the 2d state, let's say we have the up and down
arrows as trigger_p_x and trigger_n_x respectively, and right and left arrows as trigger_p_y and
trigger_n_y respectively, we would have the following states:

| Trigger State |         Vector         |  Normalized  | Magnitude | Direction (Assuming +x is 0*) |
|:-------------:|:----------------------:|:------------:|:---------:|:-----------------------------:| 
| {0, 0, 0, 0}  |  {0.f, 0.f, 0.f, 0.f}  |  {0.f, 0.f}  |     0     |              0*               |
| {0, 0, 0, 1}  | {0.f, 0.f, 0.f, -1.f}  | {0.f, -1.f}  |     1     |              90*              |
| {0, 0, 1, 0}  |  {0.f, 0.f, 1.f, 0.f}  |  {0.f, 1.f}  |     1     |             270*              |
| {0, 0, 1, 1}  | {0.f, 0.f, 1.f, -1.f}  |  {0.f, 0.f}  |     0     |              0*               |
| {0, 1, 0, 0}  | {0.f, -1.f, 0.f, 0.f}  | {-1.f, 0.f}  |     1     |             180*              |
| {0, 1, 0, 1}  | {0.f, -1.f, 0.f, -1.f} | {-1.f, -1.f} |  sqrt(2)  |             135*              |
| {0, 1, 1, 0}  | {0.f, -1.f, 1.f, 0.f}  | {-1.f, 1.f}  |  sqrt(2)  |             225*              |
| {0, 1, 1, 1}  | {0.f, -1.f, 1.f, -1.f} | {-1.f, 0.f}  |     1     |             180*              |
| {1, 0, 0, 0}  |  {1.f, 0.f, 0.f, 0.f}  |  {1.f, 0.f}  |     1     |              0*               |
| {1, 0, 0, 1}  | {1.f, 0.f, 0.f, -1.f}  | {1.f, -1.f}  |  sqrt(2)  |              45*              |
| {1, 0, 1, 0}  |  {1.f, 0.f, 1.f, 0.f}  |  {1.f, 1.f}  |  sqrt(2)  |             315*              |
| {1, 0, 1, 1}  | {1.f, 0.f, 1.f, -1.f}  |  {1.f, 0.f}  |     1     |              0*               |
| {1, 1, 0, 0}  | {1.f, -1.f, 0.f, 0.f}  |  {0.f, 0.f}  |     0     |              0*               |
| {1, 1, 0, 1}  | {1.f, -1.f, 0.f, -1.f} | {0.f, -1.f}  |     1     |              90*              |
| {1, 1, 1, 0}  | {1.f, -1.f, 1.f, 0.f}  |  {0.f, 1.f}  |     1     |             270*              |
| {1, 1, 1, 1}  | {1.f, -1.f, 1.f, -1.f} |  {0.f, 0.f}  |     0     |              0*               |

So in some scenarios, we might want the vector. In some scenarios, we might only want the normalized
vector. In some scenarios, we might only care about the magnitude and direction. But in order to not
limit ourselves, we don't want to have to choose. Therefore, we should be tracking everything.

These would look like:
```C++
struct ZeroDimensionAxis
{
    SDL_Scancode trigger;// axis input, like an SDL_Scancode, or whatever joysticks/triggers/mouse inputs use
    float v; // normalized value populated by the trigger value. 0 means completely absent, 1 means fully activated
};

struct OneDimensionAxis
{
    SDL_Scancode trigger_p; // positive axis input
    SDL_Scancode trigger_n; // negative axis input
    glm::vec2 v; // normalized values. x -> trigger_n, y -> trigger_p
    float normalize(); // normalize
};

struct TwoDimensionAxis
{
    SDL_Scancode trigger_p_x;
    SDL_Scancode trigger_n_x;
    SDL_Scancode trigger_p_y;
    SDL_Scancode trigger_n_y;
    glm::vec4 v; // normalized values. x -> trigger_p_x, y -> trigger_n_x, u -> trigger_p_y, v -> trigger_n_y
    glm::vec2 normalize(); // normalizes the vector on a unit circle
    float magnitude(); // calculates magnitude of un-normalized v
    float direction() // ?? calculates angle offset with positive y = 0/360?
};
```

We will also want an event/binding for initial press, initial release, and held. We should do
initial press and release off of SDL_Event, but when a press is recognized we should add that
scancode to a map, that maps a scancode to a struct containing information:
```C++
struct ButtonPress
{
    SDL_Scancode button;
    std::uint64_t frame_triggered;
    std::uint64_t current_frame;
    float time_pressed(); // static_cast<float>((current_frame - frame_triggered) / 1000.f);
};
```
This struct is added to the map on button press, and removed from the map on button release. Every
frame when the keyboard state is polled, if the button being pressed is in the map, the
`current_frame` field is updated. This will allow us to track how long each button is pressed. Then
the input subsystem will track all the button presses that are registered by the players, and each
button press will have a threshold. When `time_pressed()` exceeds the button threshold, the event is
called.

This way, we can have the axes for analog values like joysticks, WASD, arrow keys, etc. and we also
get a discrete event for when a button is pressed, a discrete event for when a button is released,
and a discrete event for the first time a button is pressed for a specified amount of time.