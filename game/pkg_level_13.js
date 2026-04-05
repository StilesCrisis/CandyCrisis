
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
      var PACKAGE_NAME = '/Users/johnstiles/CandyCrisis/build-web/pkg_level_13.data';
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
Module['FS_createPath']("/CandyCrisisResources/PICT_5013", "SiriRemote", true, true);
Module['FS_createPath']("/CandyCrisisResources/PICT_5013", "Web", true, true);
Module['FS_createPath']("/CandyCrisisResources/PICT_5013", "iPad", true, true);

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
          Module['removeRunDependency']('datafile_/Users/johnstiles/CandyCrisis/build-web/pkg_level_13.data');
      }
      Module['addRunDependency']('datafile_/Users/johnstiles/CandyCrisis/build-web/pkg_level_13.data');

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
    loadPackage({"files": [{"filename": "/CandyCrisisResources/PICT_5013/.DS_Store", "start": 0, "end": 6148}, {"filename": "/CandyCrisisResources/PICT_5013/Backdrop.png", "start": 6148, "end": 494088}, {"filename": "/CandyCrisisResources/PICT_5013/BackdropBlank.png", "start": 494088, "end": 588948}, {"filename": "/CandyCrisisResources/PICT_5013/BackdropCandies.png", "start": 588948, "end": 719904}, {"filename": "/CandyCrisisResources/PICT_5013/BackdropCandyPop.png", "start": 719904, "end": 847139}, {"filename": "/CandyCrisisResources/PICT_5013/Board.png", "start": 847139, "end": 847574}, {"filename": "/CandyCrisisResources/PICT_5013/BoardGrays.png", "start": 847574, "end": 847672}, {"filename": "/CandyCrisisResources/PICT_5013/BoardNext.png", "start": 847672, "end": 851626}, {"filename": "/CandyCrisisResources/PICT_5013/BoardScore.png", "start": 851626, "end": 853343}, {"filename": "/CandyCrisisResources/PICT_5013/N-1A.png", "start": 853343, "end": 920200}, {"filename": "/CandyCrisisResources/PICT_5013/N-2A.png", "start": 920200, "end": 987057}, {"filename": "/CandyCrisisResources/PICT_5013/N-3A.png", "start": 987057, "end": 1053914}, {"filename": "/CandyCrisisResources/PICT_5013/N-4A.png", "start": 1053914, "end": 1118547}, {"filename": "/CandyCrisisResources/PICT_5013/N-5A.png", "start": 1118547, "end": 1182472}, {"filename": "/CandyCrisisResources/PICT_5013/N-7A.png", "start": 1182472, "end": 1247685}, {"filename": "/CandyCrisisResources/PICT_5013/N-7B.png", "start": 1247685, "end": 1312933}, {"filename": "/CandyCrisisResources/PICT_5013/N-7C.png", "start": 1312933, "end": 1379790}, {"filename": "/CandyCrisisResources/PICT_5013/N-7D.png", "start": 1379790, "end": 1445038}, {"filename": "/CandyCrisisResources/PICT_5013/S-2A.png", "start": 1445038, "end": 1508963}, {"filename": "/CandyCrisisResources/PICT_5013/Score.png", "start": 1508963, "end": 1516794}, {"filename": "/CandyCrisisResources/PICT_5013/ScoreInnerMask.png", "start": 1516794, "end": 1517892}, {"filename": "/CandyCrisisResources/PICT_5013/ScoreOuterMask.png", "start": 1517892, "end": 1519021}, {"filename": "/CandyCrisisResources/PICT_5013/SiriRemote/BackdropDevice.png", "start": 1519021, "end": 1645282}, {"filename": "/CandyCrisisResources/PICT_5013/SiriRemote/BackdropHandCenter.png", "start": 1645282, "end": 1777935}, {"filename": "/CandyCrisisResources/PICT_5013/SiriRemote/BackdropHandClick1.png", "start": 1777935, "end": 1910513}, {"filename": "/CandyCrisisResources/PICT_5013/SiriRemote/BackdropHandClick2.png", "start": 1910513, "end": 2044354}, {"filename": "/CandyCrisisResources/PICT_5013/SiriRemote/BackdropHandDown1.png", "start": 2044354, "end": 2176386}, {"filename": "/CandyCrisisResources/PICT_5013/SiriRemote/BackdropHandDown2.png", "start": 2176386, "end": 2308721}, {"filename": "/CandyCrisisResources/PICT_5013/SiriRemote/BackdropHandLeft1.png", "start": 2308721, "end": 2441719}, {"filename": "/CandyCrisisResources/PICT_5013/SiriRemote/BackdropHandLeft2.png", "start": 2441719, "end": 2574342}, {"filename": "/CandyCrisisResources/PICT_5013/SiriRemote/BackdropHandRight1.png", "start": 2574342, "end": 2707034}, {"filename": "/CandyCrisisResources/PICT_5013/SiriRemote/BackdropHandRight2.png", "start": 2707034, "end": 2840608}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropDevice.png", "start": 2840608, "end": 2944225}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandCenter.png", "start": 2944225, "end": 3054345}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandClick1.png", "start": 3054345, "end": 3164435}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandClick2.png", "start": 3164435, "end": 3277266}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandDown1.png", "start": 3277266, "end": 3387669}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandDown2.png", "start": 3387669, "end": 3498787}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandLeft1.png", "start": 3498787, "end": 3608454}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandLeft2.png", "start": 3608454, "end": 3717975}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandRight1.png", "start": 3717975, "end": 3827602}, {"filename": "/CandyCrisisResources/PICT_5013/Web/BackdropHandRight2.png", "start": 3827602, "end": 3937320}, {"filename": "/CandyCrisisResources/PICT_5013/iPad/BackdropDevice.png", "start": 3937320, "end": 4042993}, {"filename": "/CandyCrisisResources/PICT_5013/iPad/BackdropHandCenter.png", "start": 4042993, "end": 4152927}, {"filename": "/CandyCrisisResources/PICT_5013/iPad/BackdropHandClick1.png", "start": 4152927, "end": 4262847}, {"filename": "/CandyCrisisResources/PICT_5013/iPad/BackdropHandClick2.png", "start": 4262847, "end": 4375368}, {"filename": "/CandyCrisisResources/PICT_5013/iPad/BackdropHandDown1.png", "start": 4375368, "end": 4485781}, {"filename": "/CandyCrisisResources/PICT_5013/iPad/BackdropHandDown2.png", "start": 4485781, "end": 4596851}, {"filename": "/CandyCrisisResources/PICT_5013/iPad/BackdropHandLeft1.png", "start": 4596851, "end": 4706155}, {"filename": "/CandyCrisisResources/PICT_5013/iPad/BackdropHandLeft2.png", "start": 4706155, "end": 4815247}, {"filename": "/CandyCrisisResources/PICT_5013/iPad/BackdropHandRight1.png", "start": 4815247, "end": 4924588}, {"filename": "/CandyCrisisResources/PICT_5013/iPad/BackdropHandRight2.png", "start": 4924588, "end": 5034277}], "remote_package_size": 5034277});

  })();
