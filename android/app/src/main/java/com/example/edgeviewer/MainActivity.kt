package com.example.edgeviewer

import android.Manifest
import android.app.Activity
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.Bundle
import android.util.Base64
import android.widget.Button
import android.widget.ImageView
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import java.io.ByteArrayOutputStream
import java.io.InputStream

class MainActivity : AppCompatActivity() {
    companion object {
        init { System.loadLibrary("edgeproc") }
    }

    external fun processFrameRGBA(input: ByteArray, width: Int, height: Int): ByteArray

    private val requestPermissionLauncher =
        registerForActivityResult(ActivityResultContracts.RequestPermission()) { granted ->
            if (!granted) {
                // permission denied, close
                finish()
            }
        }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            requestPermissionLauncher.launch(Manifest.permission.CAMERA)
        }

        val btnProcess = findViewById<Button>(R.id.btnProcess)
        val imgView = findViewById<ImageView>(R.id.imageView)

        // Load sample PNG packaged in assets (web/sample_processed.png) and send to native lib for processing
        btnProcess.setOnClickListener {
            val ins: InputStream = assets.open("sample_input.png")
            val bmp = BitmapFactory.decodeStream(ins)
            val width = bmp.width
            val height = bmp.height
            val buffer = java.nio.ByteBuffer.allocate(width * height * 4)
            bmp.copyPixelsToBuffer(buffer)
            val inputBytes = buffer.array()
            val outBytes = processFrameRGBA(inputBytes, width, height)
            val outBmp = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)
            outBmp.copyPixelsFromBuffer(java.nio.ByteBuffer.wrap(outBytes))
            imgView.setImageBitmap(outBmp)

            // also save base64 into log / clipboard - simple demo:
            val baos = ByteArrayOutputStream()
            outBmp.compress(Bitmap.CompressFormat.PNG, 100, baos)
            val b64 = Base64.encodeToString(baos.toByteArray(), Base64.NO_WRAP)
        }
    }
}
