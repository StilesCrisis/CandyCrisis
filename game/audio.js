// CandyCrisisAudio — web-side audio for the Emscripten build.
//
// Sound effects: Web Audio API.  Each WAV is decoded once at startup into an
// AudioBuffer; playback uses AudioBufferSourceNode with playbackRate for pitch
// and StereoPannerNode for stereo placement.
//
// Music: libopenmpt linked into the game WASM renders PCM into two Float32
// buffers (left + right) that a ScriptProcessorNode drains on each audio
// callback.  This gives us full libopenmpt tempo-factor control (FastMusic /
// SlowMusic) that SDL_mixer could not provide.

window.CandyCrisisAudio = (function () {
    var ctx        = null;   // AudioContext
    var sfxBufs    = [];     // AudioBuffer[], indexed by sound slot
    var musicGain  = null;   // GainNode  (controls music volume + mute)
    var musicNode  = null;   // ScriptProcessorNode  (pulls PCM from WASM)
    var leftPtr    = 0;      // Pre-allocated WASM heap buffer (left channel)
    var rightPtr   = 0;      // Pre-allocated WASM heap buffer (right channel)

    // -----------------------------------------------------------------------
    // Initialise — called once from C++ InitSound() via EM_ASM.
    // -----------------------------------------------------------------------
    function init() {
        ctx = new AudioContext();

        // Music chain: ScriptProcessorNode → GainNode → destination.
        // The node stays permanently connected; the C++ render function
        // outputs silence whenever no libopenmpt module is loaded or music
        // is paused, so there is no need to connect/disconnect it at runtime.
        musicGain = ctx.createGain();
        musicGain.gain.value = 0.75;   // matches FMOD default
        musicGain.connect(ctx.destination);

        // Buffer size 4096 gives ~85 ms latency at 48 kHz — acceptable for
        // background music.  ScriptProcessorNode runs on the main thread.
        var bufferBytes = 4096 * 4;  // 4096 frames × 4 bytes per float
        leftPtr  = Module._malloc(bufferBytes);
        rightPtr = Module._malloc(bufferBytes);

        musicNode = ctx.createScriptProcessor(4096, 0, 2);
        musicNode.onaudioprocess = function (e) {
            var frames = e.outputBuffer.length;
            Module._CandyCrisisAudio_RenderMusic(leftPtr, rightPtr, frames);

            e.outputBuffer.getChannelData(0).set(
                new Float32Array(Module.HEAPF32.buffer, leftPtr,  frames));
            e.outputBuffer.getChannelData(1).set(
                new Float32Array(Module.HEAPF32.buffer, rightPtr, frames));
        };
        musicNode.connect(musicGain);
    }

    // -----------------------------------------------------------------------
    // Load one WAV sound from the Emscripten virtual FS into an AudioBuffer.
    // Called for each sound slot during InitSound().  Decoding is async but
    // completes well before any sound is played.
    // -----------------------------------------------------------------------
    function loadSound(index, path) {
        try {
            var data = FS.readFile(path);          // Uint8Array from Emscripten FS
            var copy = data.buffer.slice(0);       // detach from Emscripten heap
            ctx.decodeAudioData(copy).then(function (buf) {
                sfxBufs[index] = buf;
            });
        } catch (e) {
            // Missing sound — silently skip, matches original behaviour.
        }
    }

    // -----------------------------------------------------------------------
    // Play a sound effect with pitch and stereo pan.
    //   pitchRatio : (16 + freq) / 16  — matches FMOD formula
    //   pan        : -1 = full left, 0 = centre, +1 = full right
    // -----------------------------------------------------------------------
    function playSound(which, pitchRatio, pan) {
        if (!ctx || !sfxBufs[which]) return;
        if (ctx.state === 'suspended') ctx.resume();

        var src    = ctx.createBufferSource();
        src.buffer = sfxBufs[which];
        src.playbackRate.value = pitchRatio;

        var panner     = ctx.createStereoPanner();
        panner.pan.value = pan;

        src.connect(panner);
        panner.connect(ctx.destination);
        src.start(0);
    }

    // Called from touchend / keydown handlers in shell.html.
    // touchend is the most broadly trusted gesture for AudioContext.resume() —
    // iOS Safari in particular does not honour touchstart or pointerdown for audio.
    function resumeAudio() {
        if (ctx && ctx.state === 'suspended') ctx.resume();
    }

    // vol: 0.0 = mute, 0.75 = normal (matches FMOD).
    // Pause/resume and start/stop are handled on the C++ side: the render
    // function outputs silence when no module is loaded or music is paused.
    function setMusicVolume(vol) {
        if (!musicGain) return;
        if (vol > 0 && ctx.state === 'suspended') ctx.resume();
        musicGain.gain.value = vol;
    }

    // Expose AudioContext sample rate so C++ can pass the correct rate to
    // openmpt_module_read_float_stereo().
    function getSampleRate() {
        return ctx ? ctx.sampleRate : 44100;
    }

    return {
        init,
        loadSound,
        playSound,
        setMusicVolume,
        getSampleRate,
        resumeAudio,
    };
}());
