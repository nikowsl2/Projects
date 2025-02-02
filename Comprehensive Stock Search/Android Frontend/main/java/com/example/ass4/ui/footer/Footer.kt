// In Footer.kt
package com.example.ass4.ui.components

import android.content.Intent
import android.net.Uri
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.wrapContentWidth
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.style.TextDecoration
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

@Composable
fun Footer() {
    val context = LocalContext.current
    val modifier = Modifier
        .padding(top = 10.dp)
        .fillMaxWidth()
        .wrapContentWidth(Alignment.CenterHorizontally)
        .clickable {
            val intent = Intent(Intent.ACTION_VIEW, Uri.parse("https://finnhub.io/"))
            context.startActivity(intent)
        }
    Text(
        text = "Powered by Finnhub.io",
        color = Color.Gray,
        fontSize = 12.sp,
        modifier = modifier,
        textDecoration = TextDecoration.None
    )
}
