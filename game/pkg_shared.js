
  var Module = typeof Module != 'undefined' ? Module : {};

  if (!Module['expectedDataFileDownloads']) Module['expectedDataFileDownloads'] = 0;
  Module['expectedDataFileDownloads']++;
  (() => {
    // Do not attempt to redownload the virtual filesystem data when in a pthread or a Wasm Worker context.
    var isPthread = typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD;
    var isWasmWorker = typeof ENVIRONMENT_IS_WASM_WORKER != 'undefined' && ENVIRONMENT_IS_WASM_WORKER;
    if (isPthread || isWasmWorker) return;
    var isNode = globalThis.process && globalThis.process.versions && globalThis.process.versions.node && globalThis.process.type != 'renderer';
    async function loadPackage(metadata) {

      var PACKAGE_PATH = '';
      if (typeof window === 'object') {
        PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.substring(0, window.location.pathname.lastIndexOf('/')) + '/');
      } else if (typeof process === 'undefined' && typeof location !== 'undefined') {
        // web worker
        PACKAGE_PATH = encodeURIComponent(location.pathname.substring(0, location.pathname.lastIndexOf('/')) + '/');
      }
      var PACKAGE_NAME = '/Users/johnstiles/CandyCrisis/build-web/pkg_shared.data';
      var REMOTE_PACKAGE_BASE = 'pkg_shared.data';
      var REMOTE_PACKAGE_NAME = Module['locateFile'] ? Module['locateFile'](REMOTE_PACKAGE_BASE, '') : REMOTE_PACKAGE_BASE;
      var REMOTE_PACKAGE_SIZE = metadata['remote_package_size'];

      async function fetchRemotePackage(packageName, packageSize) {
        if (isNode) {
          var contents = require('fs').readFileSync(packageName);
          return new Uint8Array(contents).buffer;
        }
        if (!Module['dataFileDownloads']) Module['dataFileDownloads'] = {};
        try {
          var response = await fetch(packageName);
        } catch (e) {
          throw new Error(`Network Error: ${packageName}`, {e});
        }
        if (!response.ok) {
          throw new Error(`${response.status}: ${response.url}`);
        }

        const chunks = [];
        const headers = response.headers;
        const total = Number(headers.get('Content-Length') || packageSize);
        let loaded = 0;

        Module['setStatus'] && Module['setStatus']('Downloading data...');
        const reader = response.body.getReader();

        while (1) {
          var {done, value} = await reader.read();
          if (done) break;
          chunks.push(value);
          loaded += value.length;
          Module['dataFileDownloads'][packageName] = {loaded, total};

          let totalLoaded = 0;
          let totalSize = 0;

          for (const download of Object.values(Module['dataFileDownloads'])) {
            totalLoaded += download.loaded;
            totalSize += download.total;
          }

          Module['setStatus'] && Module['setStatus'](`Downloading data... (${totalLoaded}/${totalSize})`);
        }

        const packageData = new Uint8Array(chunks.map((c) => c.length).reduce((a, b) => a + b, 0));
        let offset = 0;
        for (const chunk of chunks) {
          packageData.set(chunk, offset);
          offset += chunk.length;
        }
        return packageData.buffer;
      }

      var fetchPromise;
      var fetched = Module['getPreloadedPackage'] && Module['getPreloadedPackage'](REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE);

      if (!fetched) {
        // Note that we don't use await here because we want to execute the
        // the rest of this function immediately.
        fetchPromise = fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE);
      }

    async function runWithFS(Module) {

      function assert(check, msg) {
        if (!check) throw new Error(msg);
      }
Module['FS_createPath']("/", "CandyCrisisResources", true, true);

    for (var file of metadata['files']) {
      var name = file['filename']
      Module['addRunDependency'](`fp ${name}`);
    }

      async function processPackageData(arrayBuffer) {
        assert(arrayBuffer, 'Loading data file failed.');
        assert(arrayBuffer.constructor.name === ArrayBuffer.name, 'bad input to processPackageData ' + arrayBuffer.constructor.name);
        var byteArray = new Uint8Array(arrayBuffer);
        var curr;
        // Reuse the bytearray from the XHR as the source for file reads.
          for (var file of metadata['files']) {
            var name = file['filename'];
            var data = byteArray.subarray(file['start'], file['end']);
            // canOwn this data in the filesystem, it is a slice into the heap that will never change
        Module['FS_createDataFile'](name, null, data, true, true, true);
        Module['removeRunDependency'](`fp ${name}`);
          }
          Module['removeRunDependency']('datafile_/Users/johnstiles/CandyCrisis/build-web/pkg_shared.data');
      }
      Module['addRunDependency']('datafile_/Users/johnstiles/CandyCrisis/build-web/pkg_shared.data');

      if (!Module['preloadResults']) Module['preloadResults'] = {};

      Module['preloadResults'][PACKAGE_NAME] = {fromCache: false};
      if (!fetched) {
        fetched = await fetchPromise;
      }
      processPackageData(fetched);

    }
    if (Module['calledRun']) {
      runWithFS(Module);
    } else {
      if (!Module['preRun']) Module['preRun'] = [];
      Module['preRun'].push(runWithFS); // FS is not initialized yet, wait for it
    }

    }
    loadPackage({"files": [{"filename": "/CandyCrisisResources/CandyCrisisLevels.txt", "start": 0, "end": 816}, {"filename": "/CandyCrisisResources/PICT_200.png", "start": 816, "end": 627770}, {"filename": "/CandyCrisisResources/PICT_201.png", "start": 627770, "end": 671837}, {"filename": "/CandyCrisisResources/PICT_202.png", "start": 671837, "end": 686723}, {"filename": "/CandyCrisisResources/PICT_206.png", "start": 686723, "end": 998427}, {"filename": "/CandyCrisisResources/PICT_207.png", "start": 998427, "end": 1126044}, {"filename": "/CandyCrisisResources/PICT_250.png", "start": 1126044, "end": 1136701}, {"filename": "/CandyCrisisResources/PICT_251.png", "start": 1136701, "end": 1154410}, {"filename": "/CandyCrisisResources/PICT_252.png", "start": 1154410, "end": 1166847}, {"filename": "/CandyCrisisResources/PICT_254.png", "start": 1166847, "end": 1170749}, {"filename": "/CandyCrisisResources/PICT_255.png", "start": 1170749, "end": 1177415}, {"filename": "/CandyCrisisResources/PICT_256.png", "start": 1177415, "end": 1179529}, {"filename": "/CandyCrisisResources/PICT_257.png", "start": 1179529, "end": 1189037}, {"filename": "/CandyCrisisResources/PICT_258.png", "start": 1189037, "end": 1231225}, {"filename": "/CandyCrisisResources/PICT_259.png", "start": 1231225, "end": 1231315}, {"filename": "/CandyCrisisResources/PICT_300.png", "start": 1231315, "end": 1394423}, {"filename": "/CandyCrisisResources/PICT_302.png", "start": 1394423, "end": 1506090}, {"filename": "/CandyCrisisResources/PICT_303.jpg", "start": 1506090, "end": 2046466}, {"filename": "/CandyCrisisResources/PICT_400.png", "start": 2046466, "end": 2046893}, {"filename": "/CandyCrisisResources/PICT_401.png", "start": 2046893, "end": 2155501}, {"filename": "/CandyCrisisResources/PICT_402.png", "start": 2155501, "end": 2181000}, {"filename": "/CandyCrisisResources/PICT_403.png", "start": 2181000, "end": 2622393}, {"filename": "/CandyCrisisResources/PICT_404.png", "start": 2622393, "end": 2677001}, {"filename": "/CandyCrisisResources/PICT_500.png", "start": 2677001, "end": 2699771}, {"filename": "/CandyCrisisResources/PICT_501.png", "start": 2699771, "end": 2742474}, {"filename": "/CandyCrisisResources/PICT_502.png", "start": 2742474, "end": 2776971}, {"filename": "/CandyCrisisResources/PICT_503.png", "start": 2776971, "end": 2820624}, {"filename": "/CandyCrisisResources/PICT_504.png", "start": 2820624, "end": 2863365}, {"filename": "/CandyCrisisResources/PICT_510.png", "start": 2863365, "end": 2864266}, {"filename": "/CandyCrisisResources/PICT_511.png", "start": 2864266, "end": 2867923}, {"filename": "/CandyCrisisResources/PICT_512.png", "start": 2867923, "end": 2875736}, {"filename": "/CandyCrisisResources/PICT_513.png", "start": 2875736, "end": 2886897}, {"filename": "/CandyCrisisResources/PICT_514.png", "start": 2886897, "end": 2902430}, {"filename": "/CandyCrisisResources/PICT_520.png", "start": 2902430, "end": 2965978}, {"filename": "/CandyCrisisResources/PICT_530.png", "start": 2965978, "end": 2969273}, {"filename": "/CandyCrisisResources/PICT_531.png", "start": 2969273, "end": 2972638}, {"filename": "/CandyCrisisResources/PICT_532.png", "start": 2972638, "end": 2976538}, {"filename": "/CandyCrisisResources/PICT_533.png", "start": 2976538, "end": 2980626}, {"filename": "/CandyCrisisResources/PICT_534.png", "start": 2980626, "end": 2983596}, {"filename": "/CandyCrisisResources/PICT_535.png", "start": 2983596, "end": 2986667}, {"filename": "/CandyCrisisResources/PICT_536.png", "start": 2986667, "end": 2988330}, {"filename": "/CandyCrisisResources/PICT_537.png", "start": 2988330, "end": 2991176}, {"filename": "/CandyCrisisResources/PICT_538.png", "start": 2991176, "end": 2993994}, {"filename": "/CandyCrisisResources/PICT_539.png", "start": 2993994, "end": 2997608}, {"filename": "/CandyCrisisResources/PICT_540.png", "start": 2997608, "end": 3000024}, {"filename": "/CandyCrisisResources/PICT_541.png", "start": 3000024, "end": 3003108}, {"filename": "/CandyCrisisResources/PICT_542.png", "start": 3003108, "end": 3005268}, {"filename": "/CandyCrisisResources/PICT_543.png", "start": 3005268, "end": 3006711}, {"filename": "/CandyCrisisResources/PICT_544.png", "start": 3006711, "end": 3017058}, {"filename": "/CandyCrisisResources/mod_128", "start": 3017058, "end": 3121878}, {"filename": "/CandyCrisisResources/mod_140", "start": 3121878, "end": 3283250}, {"filename": "/CandyCrisisResources/mod_141", "start": 3283250, "end": 3525374}, {"filename": "/CandyCrisisResources/mod_142", "start": 3525374, "end": 3914364}, {"filename": "/CandyCrisisResources/snd_128.wav", "start": 3914364, "end": 3914768}, {"filename": "/CandyCrisisResources/snd_129.wav", "start": 3914768, "end": 3917270}, {"filename": "/CandyCrisisResources/snd_130.wav", "start": 3917270, "end": 3920444}, {"filename": "/CandyCrisisResources/snd_131.wav", "start": 3920444, "end": 3928498}, {"filename": "/CandyCrisisResources/snd_132.wav", "start": 3928498, "end": 3936962}, {"filename": "/CandyCrisisResources/snd_133.wav", "start": 3936962, "end": 3938006}, {"filename": "/CandyCrisisResources/snd_134.wav", "start": 3938006, "end": 3956164}, {"filename": "/CandyCrisisResources/snd_135.wav", "start": 3956164, "end": 3959404}, {"filename": "/CandyCrisisResources/snd_136.wav", "start": 3959404, "end": 4021164}, {"filename": "/CandyCrisisResources/snd_137.wav", "start": 4021164, "end": 4051044}, {"filename": "/CandyCrisisResources/snd_138.wav", "start": 4051044, "end": 4057334}, {"filename": "/CandyCrisisResources/snd_139.wav", "start": 4057334, "end": 4060308}, {"filename": "/CandyCrisisResources/snd_140.wav", "start": 4060308, "end": 4078978}, {"filename": "/CandyCrisisResources/snd_141.wav", "start": 4078978, "end": 4081242}, {"filename": "/CandyCrisisResources/snd_142.wav", "start": 4081242, "end": 4096232}, {"filename": "/CandyCrisisResources/snd_143.wav", "start": 4096232, "end": 4139160}, {"filename": "/CandyCrisisResources/snd_144.wav", "start": 4139160, "end": 4148164}], "remote_package_size": 4148164});

  })();
