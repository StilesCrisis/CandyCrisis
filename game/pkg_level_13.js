
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
      var PACKAGE_NAME = 'pkg_level_13.data';
      var REMOTE_PACKAGE_BASE = 'pkg_level_13.data';
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
Module['FS_createPath']("/CandyCrisisResources", "PICT_5013", true, true);
Module['FS_createPath']("/CandyCrisisResources/PICT_5013", "Web", true, true);

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
          Module['removeRunDependency']('datafile_pkg_level_13.data');
      }
      Module['addRunDependency']('datafile_pkg_level_13.data');

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
    loadPackage({"files": [{"filename": "/CandyCrisisResources/PICT_5013/Backdrop.png", "start": 0, "end": 487940}, {"filename": "/CandyCrisisResources/PICT_5013/BackdropBlank.png", "start": 487940, "end": 582800}, {"filename": "/CandyCrisisResources/PICT_5013/BackdropCandies.png", "start": 582800, "end": 713756}, {"filename": "/CandyCrisisResources/PICT_5013/BackdropCandyPop.png", "start": 713756, "end": 840991}, {"filename": "/CandyCrisisResources/PICT_5013/Board.png", "start": 840991, "end": 841426}, {"filename": "/CandyCrisisResources/PICT_5013/BoardGrays.png", "start": 841426, "end": 841524}, {"filename": "/CandyCrisisResources/PICT_5013/BoardNext.png", "start": 841524, "end": 845478}, {"filename": "/CandyCrisisResources/PICT_5013/BoardScore.png", "start": 845478, "end": 847195}, {"filename": "/CandyCrisisResources/PICT_5013/N-1A.png", "start": 847195, "end": 914052}, {"filename": "/CandyCrisisResources/PICT_5013/N-2A.png", "start": 914052, "end": 980909}, {"filename": "/CandyCrisisResources/PICT_5013/N-3A.png", "start": 980909, "end": 1047766}, {"filename": "/CandyCrisisResources/PICT_5013/N-4A.png", "start": 1047766, "end": 1112399}, {"filename": "/CandyCrisisResources/PICT_5013/N-5A.png", "start": 1112399, "end": 1176324}, {"filename": "/CandyCrisisResources/PICT_5013/N-7A.png", "start": 1176324, "end": 1241537}, {"filename": "/CandyCrisisResources/PICT_5013/N-7B.png", "start": 1241537, "end": 1306785}, {"filename": "/CandyCrisisResources/PICT_5013/N-7C.png", "start": 1306785, "end": 1373642}, {"filename": "/CandyCrisisResources/PICT_5013/N-7D.png", "start": 1373642, "end": 1438890}, {"filename": "/CandyCrisisResources/PICT_5013/S-2A.png", "start": 1438890, "end": 1502815}, {"filename": "/CandyCrisisResources/PICT_5013/Score.png", "start": 1502815, "end": 1510646}, {"filename": "/CandyCrisisResources/PICT_5013/ScoreInnerMask.png", "start": 1510646, "end": 1511744}, {"filename": "/CandyCrisisResources/PICT_5013/ScoreOuterMask.png", "start": 1511744, "end": 1512873}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropDevice.png", "start": 1512873, "end": 1616490}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandCenter.png", "start": 1616490, "end": 1726610}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandClick1.png", "start": 1726610, "end": 1836700}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandClick2.png", "start": 1836700, "end": 1949531}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandDown1.png", "start": 1949531, "end": 2059934}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandDown2.png", "start": 2059934, "end": 2171052}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandLeft1.png", "start": 2171052, "end": 2280719}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandLeft2.png", "start": 2280719, "end": 2390240}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandRight1.png", "start": 2390240, "end": 2499867}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandRight2.png", "start": 2499867, "end": 2609585}], "remote_package_size": 2609585});

  })();
